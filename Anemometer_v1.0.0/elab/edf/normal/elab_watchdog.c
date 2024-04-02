/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_watchdog.h"
#include "../../common/elab_assert.h"
#include "../../common/elab_log.h"

ELAB_TAG("Edf_WatchDog");

/* private function prototypes ---------------------------------------------- */
static elab_err_t _dev_enable(elab_device_t *me, bool status);

/* private variables -------------------------------------------------------- */
static const elab_dev_ops_t dev_ops =
{
    .enable = _dev_enable,
    NULL,
    NULL,
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
    .poll = NULL,
#endif
};

/**
 * This function register a watchdog device.
 */
void elab_watchdog_register(elab_watchdog_t *me, const char *name,
                            elab_watchdog_ops_t *ops, void *data)
{
    assert(me != NULL);
    assert(name != NULL);
    assert(ops != NULL);

    elab_device_t *device = &(me->super);

    device->ops = &dev_ops;
    device->user_data = data;

    /* register a character device */
    elab_device_attr_t attr =
    {
        .name = name,
        .sole = true,
        .type = ELAB_DEVICE_WATCHDOG,
    };
    elab_device_register(device, &attr);
}

void elab_watchdog_feed(elab_device_t *me)
{
    assert(me != NULL);

    elab_watchdog_t *wdg = (elab_watchdog_t *)me;
    assert(wdg->ops->feed != NULL);

    wdg->ops->feed(wdg);
}

void elab_watchdog_set_time(elab_device_t *me, uint32_t timeout_ms)
{
    assert(me != NULL);

    elab_watchdog_t *wdg = (elab_watchdog_t *)me;
    assert(wdg->ops->set_time != NULL);

    wdg->ops->set_time(wdg, timeout_ms);
}

/* private function --------------------------------------------------------- */
static elab_err_t _dev_enable(elab_device_t *me, bool status)
{
    assert(me != NULL);

    elab_watchdog_t *wdg = (elab_watchdog_t *)me;
    assert(wdg->ops->enable != NULL);

    wdg->ops->enable(wdg, status);

    return ELAB_OK;
}

/* ----------------------------- end of file -------------------------------- */
