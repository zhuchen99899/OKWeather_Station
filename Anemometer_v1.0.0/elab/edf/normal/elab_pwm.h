/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ELAB_PWM_H
#define ELAB_PWM_H

/* includes ----------------------------------------------------------------- */
#include "../elab_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public typedef ----------------------------------------------------------- */
typedef struct elab_pwm
{
    elab_device_t super;

    const struct elab_pwm_ops *ops;
    uint32_t frequency;
    uint8_t duty_ratio;
} elab_pwm_t;

typedef struct elab_pwm_ops
{
    void (* set_duty)(elab_pwm_t * const me, uint8_t duty_ratio);
} elab_pwm_ops_t;

/* public functions --------------------------------------------------------- */
/* For low-level driver. */
void elab_pwm_register(elab_pwm_t * const me, const char *name,
                        const elab_pwm_ops_t *ops, void *user_data);

/* For high-level code. */
void elab_pwm_set_duty(elab_device_t * const me, uint8_t duty_ratio);

#ifdef __cplusplus
}
#endif

#endif  /* ELAB_PWM_H */

/* ----------------------------- end of file -------------------------------- */
