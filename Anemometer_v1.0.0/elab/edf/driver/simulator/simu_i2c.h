
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#if defined(_WIN32) || defined(__linux__)

#ifndef DRV_I2C_H
#define DRV_I2C_H

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/* Exported typedef ----------------------------------------------------------*/

/* Exported function ---------------------------------------------------------*/
void simu_i2c_bus_export(const char *name);
void simu_i2c_new(const char *i2c_bus, uint8_t mode, uint32_t baudrate);
void simu_i2c_set_register(const char *i2c, uint32_t address, uint8_t value);

#endif /* DRV_I2C_H */

#endif

/* ----------------------------- end of file -------------------------------- */
