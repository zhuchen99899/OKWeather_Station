
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include "drv_pin.h"
#include "drv_util.h"
#include "elab/common/elab_assert.h"
#include "elab/common/elab_def.h"
#include "elab/common/elab_export.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("DriverPin");

/* private function prototype ----------------------------------------------- */
static elab_err_t _init(elab_pin_t * const me);
static elab_err_t _set_mode(elab_pin_t * const me, uint8_t mode);
static elab_err_t _get_status(elab_pin_t * const me, bool *status);
static elab_err_t _set_status(elab_pin_t * const me, bool status);

/* private variables -------------------------------------------------------- */
static const elab_pin_ops_t pin_driver_ops =
{
    .init = _init,
    .set_mode = _set_mode,
    .get_status = _get_status,
    .set_status = _set_status,
};

/* public functions --------------------------------------------------------- */
void elab_driver_pin_init(elab_pin_driver_t *me,
                            const char *name, const char *pin_name)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    assert_name(check_pin_name_valid(pin_name), pin_name);

    /* Enable the clock. */
    gpio_clock_enable(pin_name);

    me->pin_name = pin_name;
    elab_pin_register(&me->device, name, &pin_driver_ops, me);
}

/* private functions -------------------------------------------------------- */
/**
  * @brief  The PIN driver initialization function.
  * @param  me  PIN device handle.
  * @retval None.
  */
static elab_err_t _init(elab_pin_t * const me)
{
    elab_assert(me != NULL);

    elab_pin_driver_t *driver = (elab_pin_driver_t *)me->super.user_data;
    GPIO_TypeDef *port = get_port_from_name(driver->pin_name);
    uint16_t pin = get_pin_from_name(driver->pin_name);

    /* Configure GPIO pin. */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
    
    return ELAB_OK;
}

/**
  * @brief  The PIN driver set_mode function.
  * @param  me  PIN device handle.
  * @retval None.
  */
static elab_err_t _set_mode(elab_pin_t * const me, uint8_t mode)
{
    elab_assert(me != NULL);
    elab_assert(mode < PIN_MODE_MAX);

    elab_pin_driver_t *driver = (elab_pin_driver_t *)me->super.user_data;
    GPIO_TypeDef *port = get_port_from_name(driver->pin_name);
    uint16_t pin = get_pin_from_name(driver->pin_name);

    /* Configure GPIO pin. */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (mode == PIN_MODE_INPUT)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
    }
    else if (mode == PIN_MODE_INPUT_PULLUP)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
    }
    else if (mode == PIN_MODE_INPUT_PULLDOWN)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    }
    else if (mode == PIN_MODE_OUTPUT_PP)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
    }
    else if (mode == PIN_MODE_OUTPUT_OD)
    {
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
    }
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
    
    return ELAB_OK;
}

/**
  * @brief  The PIN driver set_mode function.
  * @param  me  PIN device handle.
  * @retval GPIO status.
  */
static elab_err_t _get_status(elab_pin_t * const me, bool *status)
{
    elab_assert(me != NULL);

    elab_pin_driver_t *driver = (elab_pin_driver_t *)me->super.user_data;
    GPIO_TypeDef *port = get_port_from_name(driver->pin_name);
    uint16_t pin = get_pin_from_name(driver->pin_name);

    GPIO_PinState gpip_status = HAL_GPIO_ReadPin(port, pin);
    *status = (gpip_status == GPIO_PIN_SET) ? true : false;
    
    return ELAB_OK;
}

/**
  * @brief  The PIN driver set_status function.
  * @param  me      PIN device handle.
  * @param  status  GPIO status.
  * @retval None.
  */
static elab_err_t _set_status(elab_pin_t * const me, bool status)
{
    elab_assert(me != NULL);
    
    elab_pin_driver_t *driver = (elab_pin_driver_t *)me->super.user_data;
    GPIO_TypeDef *port = get_port_from_name(driver->pin_name);
    uint16_t pin = get_pin_from_name(driver->pin_name);

    HAL_GPIO_WritePin(port, pin, (status ? GPIO_PIN_SET : GPIO_PIN_RESET));
    
    return ELAB_OK;
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
