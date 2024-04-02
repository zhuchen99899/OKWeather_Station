/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ELAB_BUTTON_H
#define ELAB_BUTTON_H

/* include ------------------------------------------------------------------ */
#include "../elab_device.h"
#include "../../3rd/qpc/include/qpc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public config ------------------------------------------------------------ */
#define ELAB_BUTTON_CLICK_TIME_MIN                  (200)
#define ELAB_BUTTON_CLICK_TIME_MAX                  (1000)
#define ELAB_BUTTON_DOUBLE_CLICK_IDLE_TIME_MAX      (300)
#define ELAB_BUTTON_LONGPRESS_TIME_MIN              ELAB_BUTTON_CLICK_TIME_MAX

/* public define ------------------------------------------------------------ */
enum
{
    ELAB_BUTTON_EVT_PRESSED = 0,
    ELAB_BUTTON_EVT_RELEASE,
    ELAB_BUTTON_EVT_CLICK,
    ELAB_BUTTON_EVT_DOUBLE_CLICK,
    ELAB_BUTTON_EVT_LONGPRESS,

    ELAB_BUTTON_EVT_MAX,
    ELAB_BUTTON_EVT_NONE = ELAB_BUTTON_EVT_MAX,
};

/* public typedef ----------------------------------------------------------- */
struct elab_button;

typedef void (* elab_button_cb_t)(struct elab_button *const me, uint8_t event_id);

typedef struct elab_button_event
{
    QEvt super;
    uint32_t button_evt;
} elab_button_event_t;

typedef struct elab_button
{
    elab_device_t super;

    uint32_t e_sig[ELAB_BUTTON_EVT_MAX];
    elab_button_cb_t cb;
    bool pressed;
    uint8_t state;
    uint32_t time_pressed;
    uint32_t time_release;
    uint8_t flag_dejitter;
    osTimerId_t timer;
    osMutexId_t mutex;

    struct elab_button_ops *ops;
} elab_button_t;

typedef struct elab_button_ops
{
    bool (* is_pressed)(elab_button_t *const me);
} elab_button_ops_t;

#define ELAB_BUTTON_CAST(_dev)        ((elab_button_t *)_dev)

/* public function ---------------------------------------------------------- */
void elab_button_register(elab_button_t *const me, const char *name,
                            elab_button_ops_t *ops, void *user_data);
void elab_button_unregister(elab_button_t *const me);

/* Button class functions */
bool elab_button_is_pressed(elab_device_t *const me);
void elab_button_set_event_signal(elab_device_t *const me, uint8_t id, uint32_t e_sig);
void elab_button_set_event_callback(elab_device_t *const me, elab_button_cb_t cb);
void elab_button_clear_event_signal(elab_device_t *const me);

#ifdef __cplusplus
}
#endif

#endif  /* ELAB_LED_H */

/* ----------------------------- end of file -------------------------------- */
