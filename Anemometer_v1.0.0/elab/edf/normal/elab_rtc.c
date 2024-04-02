/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_rtc.h"
#include "../../common/elab_assert.h"
#include "../../common/elab_log.h"

ELAB_TAG("Edf_RTC");

/* private function prototypes ---------------------------------------------- */
static elab_err_t _device_enable(elab_device_t *me, bool status);

/* private variables -------------------------------------------------------- */
/**
 * @brief  The RTC oprations function.
 */
const static elab_dev_ops_t device_ops =
{
    .enable = _device_enable,
    .read = NULL,
    .write = NULL,
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
    .poll = NULL,
#endif
};

/* public functions --------------------------------------------------------- */
/**
  * @brief  RTC bus device register
  * @param  bus         I2C bus handle.
  * @param  bus_name    Bus name.
  * @retval see elab_err_t
  */
void elab_rtc_register(elab_rtc_t *rtc, const char *name,
                        elab_rtc_ops_t *ops, void *user_data)
{
    assert(rtc != NULL);
    assert(name != NULL);
    assert(ops != NULL);

    elab_device_t *device = &(rtc->super);

    device->ops = &device_ops;
    device->user_data = user_data;

    /* register a character device */
    elab_device_attr_t attr =
    {
        .name = name,
        .sole = true,
        .type = ELAB_DEVICE_RTC,
    };
    elab_device_register(device, &attr);
}

void elab_rtc_get_time(elab_device_t *me, elab_rtc_time_t *rtc_time)
{
    assert(me != NULL);
    assert(rtc_time != NULL);

    elab_rtc_t *rtc = (elab_rtc_t *)me;

    elab_err_t ret = rtc->ops->get_time(rtc, rtc_time);
    if (ret != ELAB_OK)
    {
        elog_error("RTC get time error. Error ID: %d.", (int32_t)ret);
    }
}

void elab_rtc_set_time(elab_device_t *me, const elab_rtc_time_t *rtc_time)
{
    assert(me != NULL);
    assert(rtc_time != NULL);

    elab_rtc_t *rtc = (elab_rtc_t *)me;

    elab_err_t ret = rtc->ops->set_time(rtc, rtc_time);
    if (ret != ELAB_OK)
    {
        elog_error("RTC set time error. Error ID: %d.", (int32_t)ret);
    }
}

/* private function --------------------------------------------------------- */
static elab_err_t _device_enable(elab_device_t *me, bool status)
{
    return ELAB_OK;
}

/* ----------------------------- end of file -------------------------------- */
