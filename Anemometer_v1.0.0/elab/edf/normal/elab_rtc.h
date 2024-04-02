/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ELAB_RTC_H_
#define ELAB_RTC_H_

/* includes ----------------------------------------------------------------- */
#include "../elab_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public types ------------------------------------------------------------- */
typedef struct elab_rtc_time
{
    elab_date_t date;
    elab_time_t time;
} elab_rtc_time_t;

typedef struct elab_rtc
{
    elab_device_t super;

    const struct elab_rtc_ops *ops;
} elab_rtc_t;

typedef struct elab_rtc_ops
{
    elab_err_t (* get_time)(elab_rtc_t *rtc, elab_rtc_time_t *rtc_time);
    elab_err_t (* set_time)(elab_rtc_t *rtc, const elab_rtc_time_t *rtc_time);
} elab_rtc_ops_t;

/* public functions --------------------------------------------------------- */
void elab_rtc_register(elab_rtc_t *rtc, const char *name,
                        elab_rtc_ops_t *ops, void *user_data);
void elab_rtc_get_time(elab_device_t *me, elab_rtc_time_t *rtc_time);
void elab_rtc_set_time(elab_device_t *me, const elab_rtc_time_t *rtc_time);

#ifdef __cplusplus
}
#endif

#endif /* ELAB_RTC_H_ */

/* ----------------------------- end of file -------------------------------- */
