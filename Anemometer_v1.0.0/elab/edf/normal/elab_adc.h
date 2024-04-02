/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef __ELAB_ADC_H
#define __ELAB_ADC_H

/* includes ----------------------------------------------------------------- */
#include "../elab_device.h"
#include "../../os/cmsis_os.h"
#include "../../elib/elib_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/* private types -----------------------------------------------------------  */
typedef struct elab_adc_attr
{
    float factor;
    uint32_t interval;
    uint16_t size_cache_before;
    uint16_t size_cache_after;
    float factor_filter;
} elab_adc_attr_t;

struct elab_adc;
typedef void (* elab_adc_cache_cb_t)(struct elab_adc *const me, float *buffer);

typedef struct elab_adc
{
    elab_device_t super;

    elab_adc_cache_cb_t cb;
    float *buffer;
    elab_adc_attr_t attr;
    uint32_t value;
    bool en_auto_read;
    bool en_cache;

    osTimerId_t timer;
    elib_queue_t queue;
    const struct elab_adc_ops *ops;
} elab_adc_t;

typedef struct elab_adc_ops
{
    uint32_t (* get_value)(elab_adc_t * const me);
} elab_adc_ops_t;

/* public functions --------------------------------------------------------- */
void elab_adc_register(elab_adc_t * const me, const char *name,
                        const elab_adc_ops_t *ops, void *user_data);

float elab_adc_get_value(elab_device_t *const me);
void elab_adc_en_auto_read(elab_device_t *const me, bool status);
void elab_adc_cache_start(elab_device_t *const me,
                            elab_adc_cache_cb_t cb, float *buffer);
void elab_adc_get_attr(elab_device_t *const me, elab_adc_attr_t *attr);
void elab_adc_set_attr(elab_device_t *const me, elab_adc_attr_t *attr);
void elab_adc_set_factor(elab_device_t *const me, float factor);

#ifdef __cplusplus
}
#endif

#endif  /* __ELAB_ADC_H */

/* ----------------------------- end of file -------------------------------- */
