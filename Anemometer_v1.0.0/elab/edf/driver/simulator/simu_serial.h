
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#if defined(_WIN32) || defined(__linux__)

#ifndef DRV_SIMULATORS_H
#define DRV_SIMULATORS_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/* Exported typedef ----------------------------------------------------------*/
#define SIMU_SERIAL_MQTT_IP                 "192.168.235.1"
#define SIMU_SERIAL_MQTT_PORT               "1883"
#define SIMU_SERAIL_SLAVE_NUM_MAX           (16)
#define SIMU_SERIAL_BUFFER_MAX              (2048)

enum simu_serial_mode
{
    SIMU_SERIAL_MODE_SINGLE = 0,
    SIMU_SERIAL_MODE_UART,
    SIMU_SERIAL_MODE_MQTT,
    SIMU_SERIAL_MODE_485_M,                 /* TODO */
    SIMU_SERIAL_MODE_485_S,                 /* TODO */

    SIMU_SERIAL_MODE_MAX
};

/* Exported function ---------------------------------------------------------*/
void simu_serial_new(const char *name, uint8_t mode, uint32_t baudrate);
void simu_serial_new_pair(const char *name_one,
                            const char *name_two,
                            uint32_t baudrate);
void simu_serial_mqtt_new_pair(const char *name_one,
                                const char *name_two,
                                uint32_t baudrate);
void simu_serial_destroy(const char *name);
void simu_serial_make_rx_data(const char *name, void *buffer, uint32_t size);
void simu_serial_make_rx_data_delay(const char *name,
                                    void *buffer, uint32_t size,
                                    uint32_t delay_ms);
int32_t simu_serial_read_tx_data(const char *name,
                                    void *buffer, uint32_t size,
                                    uint32_t timeout_ms);

#endif /* DRV_SIMULATORS_H */

#endif

/* ----------------------------- end of file -------------------------------- */
