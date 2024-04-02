/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <stdlib.h>
#include <string.h>
#include "drv_util.h"
#ifdef __cplusplus
extern "C" {
#endif

/* public functions --------------------------------------------------------- */
bool check_pin_name_valid(const char *name)
{
    bool valid = true;

    if ((strlen(name) != 4) || name[1] != '.')
    {
        valid = false;
    }
    else if (!(name[0] >= 'A' && name[0] <= 'F'))
    {
        valid = false;
    }
    else if (!(name[2] >= '0' && name[2] <= '9'))
    {
        valid = false;
    }
    else if (!(name[3] >= '0' && name[3] <= '9'))
    {
        valid = false;
    }

    char *str_num = (char *)&name[2];
    int32_t pin_num = atoi(str_num);
    if (pin_num < 0 || pin_num >= 16)
    {
        valid = false;
    }

    return valid;
}

GPIO_TypeDef *get_port_from_name(const char *name)
{
    static const GPIO_TypeDef *port_table[] =
    {
        GPIOA, GPIOB, GPIOC, GPIOD, NULL, GPIOF,
    };

    return (GPIO_TypeDef *)port_table[name[0] - 'A'];
}

uint16_t get_pin_from_name(const char *name)
{
    char *str_num = (char *)&name[2];
    int32_t pin_num = atoi(str_num);

    return (uint16_t)(1 << pin_num);
}

void gpio_clock_enable(const char *name)
{
    /* Enable the clock. */
    if (get_port_from_name(name) == GPIOA)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
    else if (get_port_from_name(name) == GPIOB)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    else if (get_port_from_name(name) == GPIOC)
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    else if (get_port_from_name(name) == GPIOD)
    {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
    else if (get_port_from_name(name) == GPIOF)
    {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    }
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
