/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef DRV_UTIL_H
#define DRV_UTIL_H

/* includes ----------------------------------------------------------------- */
#include <stdbool.h>
#include <stdint.h>
#include "stm32g0xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public functions --------------------------------------------------------- */
bool check_pin_name_valid(const char *name);
GPIO_TypeDef *get_port_from_name(const char *name);
uint16_t get_pin_from_name(const char *name);
void gpio_clock_enable(const char *name);

#ifdef __cplusplus
}
#endif

#endif  /* DRV_UTIL_H */

/* ----------------------------- end of file -------------------------------- */
