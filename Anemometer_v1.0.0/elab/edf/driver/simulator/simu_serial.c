/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#if defined(__linux__) || defined(_WIN32)

/* include ------------------------------------------------------------------ */
#include "simu_serial.h"
#include "../../../common/elab_def.h"
#include "../../../elib/hash_table.h"
#include "../../../3rd/mqtt/mqttclient/mqttclient.h"
#include "../../normal/elab_serial.h"
#include "../../../common/elab_common.h"
#include "../../../common/elab_assert.h"
#include "../../../common/elab_export.h"
#include "../../../common/elab_log.h"

ELAB_TAG("SerialDriverSimuLocal");

/* Private config ----------------------------------------------------------- */
#define SIMU_HASH_TABLE_SIZE                (32)
#define SIMU_SERIAL_TOPIC_SIZE              MQTT_TOPIC_LEN_MAX

/* Private typedef ---------------------------------------------------------- */
typedef struct simu_serial
{
    elab_serial_t device;

    const char *name;
    uint8_t mode;
    uint32_t baudrate;
    bool enable;
    
    osMutexId_t mutex;
    osMessageQueueId_t queue_tx;
    osMessageQueueId_t queue_rx;
    mqtt_client_t *client;
    char topic_sub[SIMU_SERIAL_TOPIC_SIZE];

    struct simu_serial *partner;
} simu_serial_t;

/* Private function prototype ----------------------------------------------- */
static elab_err_t _enable(elab_serial_t *serial, bool status);
#if defined(__linux__) || defined(_WIN32)
static int32_t _read(elab_serial_t *serial, void *buffer, uint32_t size);
#endif
static int32_t _write(elab_serial_t *serial, const void *buffer, uint32_t size);
static elab_err_t _config(elab_serial_t *puart_dev,
                                elab_serial_config_t *pcfg);
static void msg_com_handler(void* client, message_data_t* msg);

/* Private variables -------------------------------------------------------- */
hash_table_t *ht_simu = NULL;

static elab_serial_ops_t _serial_ops =
{
    .enable = _enable,
#if defined(__linux__) || defined(_WIN32)
    .read = _read,
#endif
    .write = _write,
    .set_tx = NULL,
    .config = _config,
};

static const osMutexAttr_t mutex_attr_simu_serial =
{
    "MutexSimuSerial",
    osMutexRecursive | osMutexPrioInherit,
    NULL,
    0U
};

/* Public function ---------------------------------------------------------- */
static void simu_serial_export(void)
{
    if (ht_simu == NULL)
    {
        ht_simu = hash_table_new(SIMU_HASH_TABLE_SIZE);
        elab_assert(ht_simu != NULL);
    }
}
INIT_EXPORT(simu_serial_export, EXPORT_LEVEL_HW_INDEPNEDENT);

/**
  * @brief  The internal function for newly creating one simulated serial port.
  * @param  name        Name of the serial port.
  * @param  mode        Serial port mode. See simu_serial_mode.
  * @param  baudrate    The serial port baudrate.
  * @retval The simulated serial port handle.
  */
static simu_serial_t *_simu_serial_new(const char *name,
                                        uint8_t mode, uint32_t baudrate)
{
    simu_serial_t *serial = NULL;
    elab_assert(!hash_table_existent(ht_simu, (char *)name));
    
    elab_err_t ret = ELAB_OK;
    serial = elab_malloc(sizeof(simu_serial_t));
    elab_assert(serial != NULL);
    serial->enable = false;
    serial->baudrate = baudrate;
    serial->name = name;
    serial->partner = NULL;
    serial->mode = mode;
    serial->queue_rx = osMessageQueueNew(SIMU_SERIAL_BUFFER_MAX, 1, NULL);
    serial->mutex = osMutexNew(&mutex_attr_simu_serial);
    elab_assert(serial->queue_rx != NULL && serial->mutex != NULL);

    if (serial->mode == SIMU_SERIAL_MODE_SINGLE ||
        serial->mode == SIMU_SERIAL_MODE_UART)
    {
        serial->queue_tx = osMessageQueueNew(SIMU_SERIAL_BUFFER_MAX, 1, NULL);
        elab_assert(serial->queue_tx != NULL);
        serial->client = NULL;
    }
    else if (serial->mode == SIMU_SERIAL_MODE_MQTT)
    {
        int32_t ret_mqtt = MQTT_SUCCESS_ERROR;

        memset(serial->topic_sub, 0, SIMU_SERIAL_TOPIC_SIZE);
        serial->queue_tx = NULL;
        serial->client = mqtt_lease();
        elab_assert(serial->client != NULL);
        mqtt_set_port(serial->client, SIMU_SERIAL_MQTT_PORT);
        mqtt_set_host(serial->client, SIMU_SERIAL_MQTT_IP);
        mqtt_set_client_id(serial->client, (char *)serial->name);
        mqtt_set_clean_session(serial->client, 1);

        sprintf(serial->topic_sub, "topic_sub_%s", name);

        ret_mqtt = mqtt_connect(serial->client);
        elab_assert(ret_mqtt == MQTT_SUCCESS_ERROR);

        /* Subscribe the recieved data. */
        ret_mqtt = mqtt_subscribe(serial->client, serial->topic_sub, 
                                    QOS0, msg_com_handler);
        elab_assert(ret_mqtt == MQTT_SUCCESS_ERROR);
    }
    else
    {
        elab_assert(false);
    }

    /* Register the serial device to device framework. */
    elab_serial_attr_t attr = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;
    attr.baud_rate = baudrate;
    elab_serial_register(&serial->device, name, &_serial_ops, &attr, serial);

    /* Register the simulated serial device into the hash table. */
    ret = hash_table_add(ht_simu, (char *)name, serial);
    elab_assert(ret == ELAB_OK);

exit:
    return serial;
}

/**
  * @brief  Newly create one simulated serial port.
  * @param  name        Name of the serial port.
  * @param  mode        Serial port mode. See simu_serial_mode.
  * @param  baudrate    The serial port baudrate.
  * @retval The simulated serial port handle.
  */
void simu_serial_new(const char *name, uint8_t mode, uint32_t baudrate)
{
    elab_assert(name != NULL);
    /* TODO After 485 mode is completed, the assert will be removed. */
    elab_assert(mode == SIMU_SERIAL_MODE_SINGLE);

    _simu_serial_new(name, mode, baudrate);
}

/**
  * @brief  Destroy one simulated serial port and its related resource.
  * @param  name        Name of the serial port.
  * @retval None.
  */
void simu_serial_destroy(const char *name)
{
    simu_serial_t *serial = hash_table_get(ht_simu, (char *)name);
    elab_assert(serial != NULL);
    elab_serial_t *dev_serial = (elab_serial_t *)elab_device_find(name);
    elab_assert(dev_serial != NULL);
    elab_serial_unregister(dev_serial);

    osStatus_t ret_os = osOK;
    ret_os = osMessageQueueDelete(serial->queue_rx);
    elab_assert(ret_os == osOK);
    ret_os = osMutexDelete(serial->mutex);
    elab_assert(ret_os == osOK);

    if (serial->mode == SIMU_SERIAL_MODE_SINGLE ||
        serial->mode == SIMU_SERIAL_MODE_UART)
    {
        ret_os = osMessageQueueDelete(serial->queue_tx);
        elab_assert(ret_os == osOK);
    }
    else if (serial->mode == SIMU_SERIAL_MODE_MQTT)
    {
        int32_t ret_mqtt = MQTT_SUCCESS_ERROR;

        ret_mqtt = mqtt_disconnect(serial->client);
        elab_assert(ret_mqtt == MQTT_SUCCESS_ERROR);
        ret_mqtt = mqtt_release(serial->client);
        elab_assert(ret_mqtt == MQTT_SUCCESS_ERROR);
    }
    else
    {
        elab_assert(false);
    }

    elab_assert(hash_table_remove(ht_simu, (char *)name) == ELAB_OK);
    elab_free(serial);

exit:
    return;
}

/**
  * @brief  Newly create a pair of simulated serial ports linked with each other.
  * @param  name_one    Name of the first serial port.
  * @param  name_two    Name of the second serial port.
  * @param  baudrate    The serial port baudrate.
  * @retval None.
  */
void simu_serial_new_pair(const char *name_one, const char *name_two, uint32_t baudrate)
{
    elab_assert(!hash_table_existent(ht_simu, (char *)name_one));
    elab_assert(!hash_table_existent(ht_simu, (char *)name_two));

    simu_serial_t *serial_one = NULL;
    simu_serial_t *serial_two = NULL;

    serial_one = _simu_serial_new(name_one, SIMU_SERIAL_MODE_UART, baudrate);
    elab_assert(serial_one != NULL);

    serial_two = _simu_serial_new(name_two, SIMU_SERIAL_MODE_UART, baudrate);
    elab_assert(serial_two != NULL);
    
    serial_one->partner = serial_two;
    serial_two->partner = serial_one;
    return;

    simu_serial_destroy(name_one);
}

/**
  * @brief  Newly create a pair of simulated serial ports linked with each other.
  * @param  name_one    Name of the first serial port.
  * @param  name_two    Name of the second serial port.
  * @param  baudrate    The serial port baudrate.
  * @retval None.
  */
void simu_serial_mqtt_new_pair(const char *name_one,
                                const char *name_two,
                                uint32_t baudrate)
{
    elab_assert(!hash_table_existent(ht_simu, (char *)name_one));
    elab_assert(!hash_table_existent(ht_simu, (char *)name_two));

    simu_serial_t *serial_one = NULL;
    simu_serial_t *serial_two = NULL;

    serial_one = _simu_serial_new(name_one, SIMU_SERIAL_MODE_MQTT, baudrate);
    elab_assert(serial_one != NULL);

    serial_two = _simu_serial_new(name_two, SIMU_SERIAL_MODE_MQTT, baudrate);
    elab_assert(serial_two != NULL);
    
    serial_one->partner = serial_two;
    serial_two->partner = serial_one;
    return;

    simu_serial_destroy(name_one);
}

/**
  * @brief  Add the slave serial port to master serial port.
  * @param  name        Name of the master serial port.
  * @param  name_slave  Name of the slave serial port.
  * @retval None.
  */
void simu_serial_add_slave(const char *name, const char *name_slave)
{
    (void)name;
    (void)name_slave;

    /* TODO */

    elab_assert(false);
}

/**
  * @brief  Make some rx data for the simulated serial port.
  * @param  name    Name of the serial port.
  * @param  buffer  The data buffer.
  * @param  size    The buffer size.
  * @retval None.
  */
void simu_serial_make_rx_data(const char *name, void *buffer, uint32_t size)
{
    simu_serial_t *serial = hash_table_get(ht_simu, (char *)name);
    assert_name(serial != NULL, name);
    elab_assert(serial->mode == SIMU_SERIAL_MODE_SINGLE);

#if defined(__linux__) || defined(_WIN32)
    osStatus_t ret_os = osOK;
    uint8_t *buff = (uint8_t *)buffer;
    for (uint32_t i = 0; i < size; i ++)
    {
        ret_os = osMessageQueuePut(serial->queue_rx, &buff[i], 0, osWaitForever);
        elab_assert(ret_os == osOK);
    }
#else
    dev_serial_isr_rx(&serial->device, buffer, size);
#endif

exit:
    return;
}

/**
  * @brief  Make some rx data for the simulated serial port after a few mili-second.
  * @param  name        Name of the serial port.
  * @param  buffer      The data buffer.
  * @param  size        The buffer size.
  * @param  delay_ms    The delayed time in mili-second.
  * @retval None.
  */
void simu_serial_make_rx_data_delay(const char *name,
                                    void *buffer, uint32_t size,
                                    uint32_t delay_ms)
{
    simu_serial_t *serial = hash_table_get(ht_simu, (char *)name);
    elab_assert(serial != NULL);
    elab_assert(serial->mode == SIMU_SERIAL_MODE_SINGLE);

    osDelay(delay_ms);
    simu_serial_make_rx_data(name, buffer, size);
}

/**
  * @brief  Read the tx data from simulated serial port.
  * @param  name        Name of the serial port.
  * @param  buffer      The data buffer.
  * @param  size        The buffer size.
  * @param  timeout_ms  Timeout in mili-second.
  * @retval The actual read data size.
  */
int32_t simu_serial_read_tx_data(const char *name,
                                    void *buffer, uint32_t size,
                                    uint32_t timeout_ms)
{
    simu_serial_t *serial = hash_table_get(ht_simu, (char *)name);
    elab_assert(serial != NULL);

    int32_t ret = ELAB_ERR_TIMEOUT;
    elab_assert(serial->mode == SIMU_SERIAL_MODE_SINGLE);

    ret = ELAB_ERR_TIMEOUT;
    osStatus_t ret_os = osOK;

    uint8_t *buff = (uint8_t *)buffer;
    uint32_t time_start = osKernelGetTickCount();
    uint32_t time = timeout_ms;
    for (uint32_t i = 0; i < size; i ++)
    {
        if (timeout_ms != osWaitForever && timeout_ms != 0)
        {
            if ((osKernelGetTickCount() - time_start) < timeout_ms)
            {
                time = time_start + timeout_ms - osKernelGetTickCount();
            }
            else
            {
                break;
            }
        }
        ret_os = osMessageQueueGet(serial->queue_tx, &buff[i], NULL, time);
        if (ret_os == osOK)
        {
            ret = (ret == ELAB_ERR_TIMEOUT) ? 1 : (ret + 1);
        }
        else if (ret_os == osErrorTimeout)
        {
            break;
        }
        else
        {
            elab_assert(false);
        }
    }

exit:
    return ret;
}

/* private functions -------------------------------------------------------- */
/**
  * @brief  The simulated serial port enabling function.
  * @param  serial  The pointer of platform serial port device.
  * @param  status  Enable or diable the serial port.
  * @retval See elab_err_t.
  */
static elab_err_t _enable(elab_serial_t *serial, bool status)
{
    osStatus_t ret = osOK;

    simu_serial_t *simu_serial = container_of(serial, simu_serial_t, device);
    ret = osMutexAcquire(simu_serial->mutex, osWaitForever);
    elab_assert(ret == osOK);

    simu_serial->enable = status;

    ret = osMutexRelease(simu_serial->mutex);
    elab_assert(ret == osOK);

    return ELAB_OK;
}

#if defined(__linux__) || defined(_WIN32)
/**
  * @brief  The simulated serial port read function.
  * @param  serial  The pointer of platform serial port device.
  * @param  pos     Position
  * @param  pbuf    The pointer of buffer
  * @param  size    Expected read length
  * @retval Auctual read length
  */
static int32_t _read(elab_serial_t *serial, void *pbuf, uint32_t size)
{
    osStatus_t ret = osOK;

    simu_serial_t *simu_serial = container_of(serial, simu_serial_t, device);

    uint32_t read_cnt = 0;
    uint8_t *buffer = (uint8_t *)pbuf;
    buffer[0] = 0;
    for (uint32_t i = 0; i < size; i ++)
    {
        ret = osMessageQueueGet(simu_serial->queue_rx,
                                &buffer[i], NULL, osWaitForever);
        if (ret != osOK)
        {
            break;
        }
        read_cnt ++;
    }

exit:
    return read_cnt;
}
#endif

/**
  * @brief  The simulated serial port write function.
  * @param  serial      The pointer of platform serial port device.
  * @param  pbuf        The pointer of buffer
  * @param  size        Expected write length
  * @retval Auctual write length.
  */
static int32_t _write(elab_serial_t *serial, const void *pbuf, uint32_t size)
{
    osStatus_t ret = osOK;

    simu_serial_t *simu_serial = container_of(serial, simu_serial_t, device);
    elab_assert(simu_serial->enable);

    ret = osMutexAcquire(simu_serial->mutex, osWaitForever);
    elab_assert(ret == osOK);

    if (simu_serial->mode == SIMU_SERIAL_MODE_SINGLE)
    {
        /* Write the buffer data into message queue. */
        uint8_t *buffer = (uint8_t *)pbuf;
        for (uint32_t i = 0; i < size; i ++)
        {
            ret = osMessageQueuePut(simu_serial->queue_tx,
                                    &buffer[i], 0, osWaitForever);
            elab_assert(ret == osOK);
        }
    }
    else if (simu_serial->mode == SIMU_SERIAL_MODE_UART)
    {
        simu_serial_t *partner = simu_serial->partner;
        ret = osMutexAcquire(partner->mutex, osWaitForever);
        elab_assert(ret == osOK);

        /* Write the buffer data into message queue. */
        uint8_t *buffer = (uint8_t *)pbuf;
        for (uint32_t i = 0; i < size; i ++)
        {
            ret = osMessageQueuePut(partner->queue_rx, &buffer[i], 0, osWaitForever);
            elab_assert(ret == osOK);
        }

        ret = osMutexRelease(partner->mutex);
        elab_assert(ret == osOK);
    }
    else if (simu_serial->mode == SIMU_SERIAL_MODE_MQTT)
    {
        simu_serial_t *partner = simu_serial->partner;
        ret = osMutexAcquire(partner->mutex, osWaitForever);
        elab_assert(ret == osOK);

        /* Write the buffer data into message queue. */
        mqtt_message_t msg;
        memset(&msg, 0, sizeof(msg));
        msg.qos = QOS0;
        msg.payload = (void *)pbuf;
        msg.payloadlen = size;
        int32_t ret_mqtt = mqtt_publish(partner->client,
                                        partner->topic_sub,
                                        &msg);
        elab_assert(ret_mqtt == MQTT_SUCCESS_ERROR);

        ret = osMutexRelease(partner->mutex);
        elab_assert(ret == osOK);
    }
    else
    {
        elab_assert(false);
    }

    ret = osMutexRelease(simu_serial->mutex);
    elab_assert(ret == osOK);

    elab_serial_tx_end(&simu_serial->device);

exit:
    return size;
}

/**
  * @brief  The simulated serial port config function.
  * @param  serial  The pointer of platform serial port device.
  * @param  config  The serial port configuration.
  * @retval See elab_err_t.
  */
static elab_err_t _config(elab_serial_t *serial, elab_serial_config_t *config)
{
    (void)serial;
    (void)config;

    return ELAB_OK;
}

/**
  * @brief  The MQTT handler for simulated serial.
  * @param  client  The MQTT handle.
  * @param  msg     The MQTT message.
  * @retval None.
  */
static void msg_com_handler(void *client, message_data_t *msg)
{
    (void)client;

    char *serial_name = &msg->topic_name[strlen("topic_sub_")];
    simu_serial_t *serial = hash_table_get(ht_simu, serial_name);

    if (elab_device_is_enabled(&serial->device.super))
    {
        uint8_t *load = msg->message->payload;
        for (uint32_t i = 0; i < msg->message->payloadlen; i ++)
        {
            osMessageQueuePut(serial->queue_rx, (const void *)&load[i], 0, 0);
        }
    }
}

#endif

/* ----------------------------- end of file -------------------------------- */
