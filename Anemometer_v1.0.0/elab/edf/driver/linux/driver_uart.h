/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef DRIVER_UART_H
#define DRIVER_UART_H

/* include ------------------------------------------------------------------ */
#include "../../elab_device.h"
#include "../../normal/elab_serial.h"

/* private typedef ---------------------------------------------------------- */
typedef struct driver_uart
{
    const char * name_serial;
    const char * name_gpio;
    uint32_t baudrate;

    int32_t serial_fd;
    const char * name_linux_serial;
    elab_device_t *pin_tx_switch;
    elab_serial_t device;
} driver_uart_t;

/* public function ---------------------------------------------------------- */
void driver_serial_init(driver_uart_t *me,
                        elab_serial_t *serial_dev,
                        const char *name_dev,
                        const char *name_linux_serial, const char *name_gpio,
                        uint32_t baudrate);

#endif /* DRIVER_UART_H */

/* ----------------------------- end of file -------------------------------- */
