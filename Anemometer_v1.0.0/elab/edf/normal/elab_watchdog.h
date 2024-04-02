/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ELAB_WATCHDOG_H_
#define ELAB_WATCHDOG_H_

/* includes ----------------------------------------------------------------- */
#include "../elab_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public types ------------------------------------------------------------- */
typedef struct elab_watchdog_device
{
    elab_device_t super;

    const struct elab_watchdog_ops *ops;
} elab_watchdog_t;

typedef struct elab_watchdog_ops
{
    void (* enable)(elab_watchdog_t *wdt, bool status);
    void (* feed)(elab_watchdog_t *wdt);
    void (* set_time)(elab_watchdog_t *wdt, uint32_t timeout_ms);
} elab_watchdog_ops_t;

/* public functions --------------------------------------------------------- */
void elab_watchdog_register(elab_watchdog_t *wdt, const char *name,
                            elab_watchdog_ops_t *ops, void *data);
void elab_watchdog_feed(elab_device_t *me);
void elab_watchdog_set_time(elab_device_t *me, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* ELAB_WATCHDOG_H_ */

/* ----------------------------- end of file -------------------------------- */
