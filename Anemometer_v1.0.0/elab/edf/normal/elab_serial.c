/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_serial.h"
#include "../../common/elab_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("Edf_Serial");

/* public function prototypes ----------------------------------------------- */
void elab_device_unregister(elab_device_t *me);

/* private function prototypes ---------------------------------------------- */
static elab_err_t _device_enable(elab_device_t *me, bool status);
static int32_t _device_read(elab_device_t *me,
                                uint32_t pos, void *buffer, uint32_t size);
static int32_t _device_write(elab_device_t *me,
                                uint32_t pos, const void *buffer, uint32_t size);
static void _thread_entry(void *parameter);

#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
static void _device_poll(elab_device_t *me);
#endif

/* private variables -------------------------------------------------------- */
static const elab_dev_ops_t _device_ops =
{
    .enable = _device_enable,
    .read = _device_read,
    .write = _device_write,
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
    .poll = _device_poll,
#endif
};

static const osMutexAttr_t _mutex_attr =
{
    "mutex_serial_half_duplex",
    osMutexRecursive | osMutexPrioInherit,
    NULL,
    0U 
};

static const osThreadAttr_t thread_attr_serial_rx = 
{
    .name = "ThreadSerailRx",
    .attr_bits = osThreadDetached,
    .priority = osPriorityRealtime,
    .stack_size = 2048,
};

/* public functions --------------------------------------------------------- */
/**
  * @brief  Register elab serial device to edf framework.
  * @param  serial      elab serial device handle.
  * @param  name        Serial name.
  * @param  ops         Serial device ops.
  * @param  attr        Serial device attribute.
  * @param  user_data   The pointer of private data.
  * @retval None.
  */
void elab_serial_register(elab_serial_t *serial, const char *name,
                            elab_serial_ops_t *ops,
                            elab_serial_attr_t *attr,
                            void *user_data)
{
    elab_assert(serial != NULL);
    elab_assert(name != NULL);
    elab_assert(ops != NULL);

    memset(serial, 0, sizeof(elab_serial_t));

    /* The serial class data */
    serial->ops = ops;
    if (attr == NULL)
    {
        serial->attr = (elab_serial_attr_t)ELAB_SERIAL_ATTR_DEFAULT;
    }
    else
    {
        memcpy(&serial->attr, attr, sizeof(elab_serial_attr_t));
    }

    /* The serial device initialization. */
    serial->ops = ops;
    if (serial->ops->config != NULL)
    {
        elab_serial_attr_t attr = serial->attr;
        serial->ops->config(serial, (elab_serial_config_t *)&attr);
    }
    /* If RS485 mode, set to rx mode. */
    if (serial->attr.mode == ELAB_SERIAL_MODE_HALF_DUPLEX)
    {
        serial->ops->set_tx(serial, false);
    }
    
    /* Apply buffer memory */
    serial->mutex_tx = osMutexNew(&_mutex_attr);
    elab_assert(serial->mutex_tx != NULL);
    serial->sem_tx = osSemaphoreNew(1, 0, NULL);
    elab_assert(serial->sem_tx != NULL);
    serial->queue_rx = osMessageQueueNew(serial->attr.rx_bufsz, 1, NULL);
    elab_assert(serial->queue_rx != NULL);

    /* The super class data */
    elab_device_t *device = &(serial->super);
    device->ops = &_device_ops;
    device->user_data = user_data;

    /* register a character device */
    elab_device_attr_t _dev_attr =
    {
        .name = name,
        .sole = true,
        .type = ELAB_DEVICE_UART,
    };
    if (serial->attr.mode == ELAB_SERIAL_MODE_HALF_DUPLEX)
    {
        _dev_attr.sole = false;
    }
    elab_device_register(device, &_dev_attr);

    /* Apply buffer memory */
#if defined(__linux__) || defined(_WIN32)
    serial->thread_rx = osThreadNew(_thread_entry, serial, &thread_attr_serial_rx);
    elab_assert(serial->thread_rx != NULL);
#endif
}

/**
  * @brief  Register serial device to elab device
  * @param  serial      The pointer of elab serial dev
  * @retval None.
  */
void elab_serial_unregister(elab_serial_t *serial)
{
    elab_assert(serial != NULL);
    elab_assert(!elab_device_is_enabled(ELAB_DEVICE_CAST(serial)));

    osStatus_t ret_os = osOK;

#if defined(__linux__) || defined(_WIN32)
    ret_os = osThreadTerminate(serial->thread_rx);
    elab_assert(ret_os == osOK);
    serial->thread_rx = NULL;
#endif

    elab_device_lock(ELAB_DEVICE_CAST(serial));

    /* The serial tx mutex creating. */
    ret_os = osMutexDelete(serial->mutex_tx);
    elab_assert(ret_os == osOK);
    serial->mutex_tx = NULL;

    ret_os = osSemaphoreDelete(serial->sem_tx);
    elab_assert(ret_os == osOK);
    serial->sem_tx = NULL;

    ret_os = osMessageQueueDelete(serial->queue_rx);
    elab_assert(ret_os == osOK);
    serial->queue_rx = NULL;

    elab_device_unlock(ELAB_DEVICE_CAST(serial));

    elab_device_unregister(ELAB_DEVICE_CAST(serial));
}

/**
  * @brief  The serial device tx ISR function.
  * @param  serial      elab serial device handle
  * @retval None.
  */
void elab_serial_tx_end(elab_serial_t *serial)
{
    elab_assert(serial != NULL);
    elab_assert(serial->sem_tx != NULL);

    osStatus_t ret_os = osSemaphoreRelease(serial->sem_tx);
    elab_assert(ret_os == osOK);
}

#if !defined(__linux__) && !defined(_WIN32)
/**
  * @brief  The serial device rx ISR function.
  * @param  serial      elab serial device handle.
  * @param  buffer      The buffer memory.
  * @param  size        The serial memory size.
  * @retval None.
  */
void elab_serial_isr_rx(elab_serial_t *serial, void *buffer, uint32_t size)
{
    elab_assert(serial != NULL);
    elab_assert(buffer != NULL);
    elab_assert(size != 0);

    osStatus_t ret_os = osOK;
    uint8_t *buff = (uint8_t *)buffer;
    if (!device_is_test_mode(&serial->parent))
    {
        if (elab_device_is_enabled(&serial->super))
        {
            for (uint32_t i = 0; i < size; i ++)
            {
                ret_os = osMessageQueuePut(serial->queue_rx, &buff[i], 0, 0);
                elab_assert(ret_os == osOK);
            }
        }
    }
}
#endif

/**
  * @brief  elab serial device write function.
  * @param  me      The elab device handle.
  * @param  buff    The pointer of buffer
  * @param  size    Expected write length
  * @retval Auctual write length or error ID.
  */
int32_t elab_serial_write(elab_device_t * const me, void *buff, uint32_t size)
{
    elab_assert(me != NULL);
    elab_assert(buff != NULL);
    elab_assert(size != 0);
    elab_assert(elab_device_is_enabled(me));

    int32_t ret = ELAB_OK;
    osStatus_t ret_os = osOK;
    elab_serial_t *serial = (elab_serial_t *)me;
    elab_assert(serial->ops != NULL);
    elab_assert(serial->ops->write != NULL);
    if (serial->attr.mode == ELAB_SERIAL_MODE_HALF_DUPLEX)
    {
        elab_assert(serial->ops->set_tx != NULL);
    }

    /* If not in testing mode. */
    if (!elab_device_is_test_mode(&serial->super))
    {
        /* Lock serial in the sending process to prevent data from being sent in
           other threads. */
        ret_os = osMutexAcquire(serial->mutex_tx, osWaitForever);
        elab_assert(ret_os == osOK);

        /* If RS485 mode, set to tx mode. */
        if (serial->attr.mode == ELAB_SERIAL_MODE_HALF_DUPLEX)
        {
            serial->ops->set_tx(serial, true);
        }

        /* Send data by the low level serial write function in non-block mode. */
        ret = serial->ops->write(serial, buff, size);
        if (ret < 0)
        {
            goto exit;
        }

        /* Wait the send process to be completed, which is released in the function
           dev_serial_tx_end. */
        ret_os = osSemaphoreAcquire(serial->sem_tx, osWaitForever);
        elab_assert(ret_os == osOK);

exit:
        /* If RS485 mode, set to rx mode. */
        if (serial->attr.mode == ELAB_SERIAL_MODE_HALF_DUPLEX)
        {
            serial->ops->set_tx(serial, false);
        }

        /* Unlock serial at the end of sending process. */
        ret_os = osMutexRelease(serial->mutex_tx);
        elab_assert(ret_os == osOK);
    }

exit1:
    return ret;
}

/**
  * @brief  elab device read function
  * @param  me      The elab device handle.
  * @param  buffer  The pointer of buffer
  * @param  size    Expected read length
  * @retval Auctual write length or error ID.
  */
int32_t elab_serial_read(elab_device_t * const me, void *buff,
                            uint32_t size, uint32_t timeout)
{
    elab_assert(me != NULL);
    elab_assert(buff != NULL);
    elab_assert(size != 0);
    elab_assert(elab_device_is_enabled(me));

    int32_t ret = ELAB_OK;
    osStatus_t ret_os = osOK;
    elab_serial_t *serial = (elab_serial_t *)me;
    elab_assert(serial->ops != NULL);
    elab_assert(serial->ops->read != NULL);

    /* If not in testing mode. */
    if (!elab_device_is_test_mode(&serial->super))
    {
        uint32_t time_start = osKernelGetTickCount();
        uint32_t time = timeout;
        for (uint32_t i = 0; i < size; i ++)
        {
            if (timeout != osWaitForever && timeout != 0)
            {
                if ((osKernelGetTickCount() - time_start) < timeout)
                {
                    time = time_start + timeout - osKernelGetTickCount();
                }
                else
                {
                    if (ret == ELAB_OK)
                    {
                        ret = ELAB_ERR_TIMEOUT;
                    }
                    break;
                }
            }

            ret_os = osMessageQueueGet(serial->queue_rx,
                                        &((uint8_t *)buff)[i], 0U, time);
            if (ret_os == osErrorTimeout)
            {
                if (ret == ELAB_OK)
                {
                    ret = ELAB_ERR_TIMEOUT;
                }
                break;
            }
            if (ret_os == osOK)
            {
                ret ++;
                if (ret >= size)
                {
                    break;
                }
                else
                {
                    continue;
                }
            }
            elab_assert(false);
        }
    }
    else
    {
#if !defined(__linux__) && !defined(_WIN32)
        /* Clear the queue for rx. */
        ret_os = osMessageQueueReset(serial->queue_rx);
        elab_assert(ret_os == osOK);
#endif
        if (timeout == osWaitForever)
        {
            while (1)
            {
                if (!elab_device_is_test_mode(&serial->super))
                {
                    break;
                }
                osDelay(1);
            }
        }
        else
        {
            osDelay(timeout);
            ret = ELAB_ERR_TIMEOUT;
        }
    }

exit:
    return ret;
}

/**
  * @brief  Set the attribute of the elab serial device
  * @param  serial      elab serial device handle
  * @param  name        Serial name
  * @param  ops         Serial device ops
  * @param  user_data   The pointer of private data
  * @retval See elab_err_t
  */
void elab_serial_set_attr(elab_device_t *me, elab_serial_attr_t *attr)
{
    elab_assert(me != NULL);
    elab_assert(attr != NULL);
    elab_assert(elab_device_is_enabled(me));

    elab_serial_t *serial = ELAB_SERIAL_CAST(me);

    elab_device_lock(serial);
    elab_assert(attr->mode == serial->attr.mode);
    elab_assert(attr->rx_bufsz == serial->attr.rx_bufsz);
    /* Set the config data of serial device. */
    if (memcmp(&serial->attr, attr, sizeof(elab_serial_attr_t)) != 0)
    {
        elab_err_t ret = serial->ops->config(serial, (elab_serial_config_t *)attr);
        if (ret == ELAB_OK)
        {
            memcpy(&serial->attr, attr, sizeof(elab_serial_attr_t));
        }
    }
    elab_device_unlock(serial);
}

/**
  * @brief  Register elab serial device to serial device
  * @param  serial      elab serial device handle
  * @param  name        Serial name
  * @param  ops         Serial device ops
  * @param  user_data   The pointer of private data
  * @retval See elab_err_t
  */
elab_serial_attr_t elab_serial_get_attr(elab_device_t *me)
{
    elab_assert(me != NULL);

    elab_serial_attr_t attr;
    elab_serial_t *serial = ELAB_SERIAL_CAST(me);

    elab_device_lock(serial);
    memcpy(&attr, &serial->attr, sizeof(elab_serial_attr_t));
    elab_device_unlock(serial);

    return attr;
}

/**
  * @brief  Set the serial device baudrate.
  * @param  dev         The device handle.
  * @param  baudrate    The serial device baudrate.
  * @retval See elab_err_t.
  */
void elab_serial_set_baudrate(elab_device_t *me, uint32_t baudrate)
{
    elab_assert(me != NULL);
    elab_assert(baudrate == 4800 || baudrate == 9600 ||
                    baudrate == 19200 || baudrate == 38400 ||
                    baudrate == 57600 || baudrate == 115200 ||
                    baudrate == 230400 || baudrate == 460800 ||
                    baudrate == 921600 || baudrate >= 1000000);

    elab_err_t ret = ELAB_OK;
    if (!elab_device_is_enabled(me))
    {
        elog_error("The serial device %s is NOT open yet.", me->attr.name);
        ret = ELAB_ERROR;
        goto exit;
    }

    elab_serial_attr_t attr = elab_serial_get_attr(me);
    if (attr.baud_rate != baudrate)
    {
        attr.baud_rate = baudrate;
        elab_serial_set_attr(me, &attr);
    }

exit:
    return;
}

/**
  * @brief  elab serial device xfer function, just for half duplex mode.
  * @param  serial      elab serial device handle
  * @param  name        Serial name
  * @param  ops         Serial device ops
  * @param  user_data   The pointer of private data
  * @retval See elab_err_t
  */
int32_t elab_serial_xfer(elab_device_t *me,
                            void *buff_tx, uint32_t size_tx,
                            void *buff_rx, uint32_t size_rx,
                            uint32_t timeout)
{
    elab_assert(me != NULL);
    elab_assert(buff_tx != NULL);
    elab_assert(buff_tx != NULL);
    elab_assert(size_tx != 0);
    elab_assert(size_rx != 0);
    elab_assert(ELAB_SERIAL_CAST(me)->ops != NULL);
    elab_assert(ELAB_SERIAL_CAST(me)->ops->write != NULL);
    elab_assert(ELAB_SERIAL_CAST(me)->ops->read != NULL);

    elab_serial_t *serial = ELAB_SERIAL_CAST(me);
    elab_assert(serial->attr.mode == ELAB_SERIAL_MODE_HALF_DUPLEX);

    uint32_t time_start = osKernelGetTickCount();
    uint32_t timeout_read = timeout;
    int32_t ret = elab_serial_write(me, buff_tx, size_tx);
    if (ret > 0)
    {
        if (timeout != osWaitForever && timeout != 0)
        {
            if (osKernelGetTickCount() >= (time_start + timeout))
            {
                ret = ELAB_ERR_TIMEOUT;
                goto exit;
            }

            timeout_read = osKernelGetTickCount() - time_start;
        }

        assert_id(ret == size_tx, ret);
        ret = elab_serial_read(me, buff_rx, size_rx, timeout_read);
    }

exit:
    return ret;
}

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  elab device enable function
  * @param  me  The elab device handle.
  * @retval See elab_err_t
  */
static elab_err_t _device_enable(elab_device_t *me, bool status)
{
    elab_err_t ret = ELAB_OK;

    elab_assert(me != NULL);

    elab_serial_t *serial = (elab_serial_t *)me;
    elab_assert(serial->ops != NULL);
    elab_assert(serial->ops->enable != NULL);

    return serial->ops->enable(serial, status);
}

/**
  * @brief  elab device read function
  * @param  me      The elab device handle.
  * @param  pos     Position
  * @param  buffer  The pointer of buffer
  * @param  size    Expected read length
  * @retval Auctual read length
  */
static int32_t _device_read(elab_device_t *me,
                            uint32_t pos, void *buffer, uint32_t size)
{
    (void)pos;

    return elab_serial_read(me, buffer, size, osWaitForever);
}

/**
  * @brief  elab device write function.
  * @param  me      The elab device handle.
  * @param  pos     Position
  * @param  buffer  The pointer of buffer
  * @param  size    Expected write length
  * @retval Auctual write length
  */
static int32_t _device_write(elab_device_t *me,
                                uint32_t pos, const void *buffer, uint32_t size)
{
    (void)pos;

    return elab_serial_write(me, (void *)buffer, size);
}


/**
  * @brief  The entry function for serial device data receiving.
  */
#if defined(__linux__) || defined(_WIN32)
static void _thread_entry(void *parameter)
{
    elab_serial_t *serial = (elab_serial_t *)parameter;
    elab_assert(serial != NULL);
    elab_assert(serial->ops != NULL);
    elab_assert(serial->ops->read != NULL);

    uint8_t data;
    int32_t ret = 0;
    osStatus_t ret_os = osOK;

    while (1)
    {
        if (elab_device_is_enabled(&serial->super) &&
            !elab_device_is_test_mode(&serial->super))
        {
            ret = serial->ops->read(serial, &data, 1);
            elab_assert(ret == 1);
            ret_os = osMessageQueuePut(serial->queue_rx, &data, 0, osWaitForever);
            elab_assert(ret_os == osOK);
        }
        else
        {
            osDelay(10);
        }
    }
}
#endif

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
