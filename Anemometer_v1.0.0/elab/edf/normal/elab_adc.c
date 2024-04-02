
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_adc.h"
#include "../../common/elab_assert.h"

ELAB_TAG("Edf_ADC");

/* private function prototype ----------------------------------------------- */
static void _timer_cb(void *argument);

/* private variables -------------------------------------------------------- */
static const osTimerAttr_t timer_attr_adc =
{
    .name = "adc_timer",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0,
};

static elab_dev_ops_t _ops =
{
    .enable = NULL,
    .read = NULL,
    .write = NULL,
};

/* public functions --------------------------------------------------------- */
/**
  * @brief  eLab adc register function.
  * @param  me          this pointer
  * @param  name        adc's name.
  * @param  ops         ops interface.
  * @param  user_data   User private data.
  * @retval None
  */
void elab_adc_register(elab_adc_t * const me, const char *name,
                        const elab_adc_ops_t *ops, void *user_data)
{
    assert(me != NULL);
    assert(name != NULL);
    assert(ops != NULL);

    me->timer = osTimerNew(_timer_cb, osTimerPeriodic, me, &timer_attr_adc);
    assert_name(me->timer != NULL, name);

    elab_device_attr_t attr =
    {
        .name = name,
        .sole = true,
        .type = ELAB_DEVICE_ADC,
    };
    elab_device_register(&me->super, &attr);
    me->super.user_data = user_data;
    me->super.ops = &_ops;

    me->ops = ops;
    me->cb = NULL;
    me->en_cache = false;
    me->en_auto_read = false;
    
    me->attr.factor = (3.3 / 4096.0);
}

/**
  * @brief  Get the eLab adc's value.
  * @param  me      this pointer
  * @retval The adc value.
  */
float elab_adc_get_value(elab_device_t *const me)
{
    assert(me != NULL);
    assert(me->attr.type == ELAB_DEVICE_ADC);
    
    elab_adc_t *adc = (elab_adc_t *)me;
    uint32_t value = adc->en_auto_read ? adc->value : adc->ops->get_value(adc);
    return (float)value * adc->attr.factor;
}

/**
  * @brief  Get the eLab adc's value.
  * @param  me      this pointer
  * @retval The adc value.
  */
void elab_adc_en_auto_read(elab_device_t *const me, bool status)
{
    assert(me != NULL);
    assert(me->attr.type == ELAB_DEVICE_ADC);

    elab_adc_t *adc = (elab_adc_t *)me;
    adc->en_auto_read = status;
}

/**
  * @brief  Start the eLab adc's data cache function.
  * @param  me      this pointer
  * @param  cb      The call back function when the data cache stops.
  * @param  buffer  The data buffer.
  * @retval None.
  */
void elab_adc_cache_start(elab_device_t *const me,
                            elab_adc_cache_cb_t cb, float *buffer)
{
    assert(me != NULL);
    assert(me->attr.type == ELAB_DEVICE_ADC);
    
    assert(cb != NULL);
    assert(buffer != NULL);

    elab_adc_t *adc = (elab_adc_t *)me;
    assert(!adc->en_cache);
    
    adc->cb = cb;
    adc->buffer = buffer;
    adc->en_cache = true;
}

/**
  * @brief  eLab adc's attribute getting function.
  * @param  me      this pointer
  * @param  attr    elab adc attibute output.
  * @retval None.
  */
void elab_adc_get_attr(elab_device_t *const me, elab_adc_attr_t *attr)
{
    assert(me != NULL);
    assert(attr != NULL);

    memcpy(attr, &me->attr, sizeof(elab_adc_attr_t));
}

/**
  * @brief  eLab adc's attribute setting function.
  * @param  me      this pointer
  * @param  attr    elab adc attibute.
  * @retval None.
  */
void elab_adc_set_attr(elab_device_t *const me, elab_adc_attr_t *attr)
{
    assert(me != NULL);
    assert(attr != NULL);

    elab_adc_t *adc = (elab_adc_t *)me;
    osStatus_t ret_os = osOK;
    bool interval_changed = false;
    if (attr->interval != adc->attr.interval)
    {
        interval_changed = true;
    }

    memcpy(&me->attr, attr, sizeof(elab_adc_attr_t));

    if (adc->en_auto_read && interval_changed)
    {
        /* Stop the old timer and restart it. */
        ret_os = osTimerStop(adc->timer);
        assert(ret_os == osOK);
        ret_os = osTimerStart(adc->timer, attr->interval);
        assert(ret_os == osOK);
        (void)ret_os;
    }
}

void elab_adc_set_factor(elab_device_t *const me, float factor)
{
    elab_adc_attr_t attr;
    elab_adc_get_attr(me, &attr);
    attr.factor = factor;
    elab_adc_set_attr(me, &attr);
}

/* private functions -------------------------------------------------------- */
/**
  * @brief  eLab ADC timer callback function
  * @param  argument - callback argument
  * @retval None
  */
static void _timer_cb(void *argument)
{
    elab_adc_t *adc = (elab_adc_t *)argument;
    
    adc->cb(adc, adc->buffer);
}

/* ----------------------------- end of file -------------------------------- */
