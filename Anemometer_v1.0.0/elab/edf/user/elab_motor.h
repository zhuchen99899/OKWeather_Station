/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ELAB_MOTOR_H
#define ELAB_MOTOR_H

/* include ------------------------------------------------------------------ */
#include "../elab_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/* private typedef ---------------------------------------------------------- */
typedef struct elab_motor
{
    elab_device_t super;

    /* private parameter. */
    uint32_t ratio;

    /* private data. */
    uint8_t state;
    float speed_cmd;
    float speed_current;
    uint32_t time_speed_update;

    struct elab_motor_ops *ops;
} elab_motor_t;

typedef struct elab_motor_ops
{
    elab_err_t (* init)(elab_motor_t *const me);
    elab_err_t (* enable)(elab_motor_t *const me, bool status);
    elab_err_t (* emg_stop)(elab_motor_t *const me);
    elab_err_t (* get_error)(elab_motor_t *const me, uint32_t *error_out);
    bool (* ready)(elab_motor_t *const me);
    elab_err_t (* set_speed)(elab_motor_t *const me, float speed);
    elab_err_t (* get_speed)(elab_motor_t *const me, float *speed);
} elab_motor_ops_t;

#define ELAB_MOTOR_CAST(_dev)           ((elab_motor_t *)_dev)

/* public function ---------------------------------------------------------- */
/* Config functions, for lua script programming. */
void elab_motor_config_ratio(const char *const name, uint32_t ratio);

/* Motor class functions */
elab_err_t elab_motor_enable(elab_device_t *const me, bool status);
elab_err_t elab_motor_emg_stop(elab_device_t *const me);
elab_err_t elab_motor_get_error(elab_device_t *const me, uint32_t *error_out);
elab_err_t elab_motor_set_speed(elab_device_t *const me, float speed);
elab_err_t elab_motor_get_speed(elab_device_t *const me, float *speed);

/* For exporting. */
void elab_motor_init(elab_motor_t *const me, const char *name,
                        elab_motor_ops_t *ops, void *user_data);

#ifdef __cplusplus
}
#endif

#endif  /* ELAB_MOTOR_H */

/* ----------------------------- end of file -------------------------------- */
