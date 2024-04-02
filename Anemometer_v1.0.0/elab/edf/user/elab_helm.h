/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ELAB_HELM_H
#define ELAB_HELM_H

/* include ------------------------------------------------------------------ */
#include "elab_def.h"
#include "elab_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/* private typedef ---------------------------------------------------------- */
typedef struct elab_helm
{
    elab_device_t super;

    uint32_t ratio;
    float speed_cmd;
    float speed_current;
} elab_helm_t;

struct elab_helm_ops
{
    elab_err_t (* init)(elab_helm_t *const me);
    bool (* ready)(elab_helm_t *const me);
    elab_err_t (* set_speed)(elab_helm_t *const me, float speed);
    elab_err_t (* get_speed)(elab_helm_t *const me, float *speed);
};

/* public function ---------------------------------------------------------- */
/* General functions */
void elab_helm_config_pos_init(const char *helm_name, float pos);
void elab_helm_config_w(const char *helm_name, float w);
void elab_helm_config_ratio(const char *helm_name, uint32_t ratio);

/* Helm class functions */
elab_err_t elab_helm_init(elab_helm_t *const me);
elab_err_t elab_helm_set_position(elab_helm_t *const me, float pos);

#ifdef __cplusplus
}
#endif

#endif  /* ELAB_HELM_H */

/* ----------------------------- end of file -------------------------------- */
