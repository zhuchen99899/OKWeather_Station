/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include "elab_button.h"
#include "../../common/elab_assert.h"

ELAB_TAG("EdfButtton");

/* private defines ---------------------------------------------------------- */
enum
{
    BUTTON_STATE_IDLE = 0,
    BUTTON_STATE_PRESSED,
    BUTTON_STATE_DOUBLE_CLICK_IDLE,
    BUTTON_STATE_DOUBLE_CLICK_PRESSED,

    BUTTON_STATE_MAX
};

/* public function prototype ------------------------------------------------ */
void elab_device_unregister(elab_device_t *me);

/* private function prototype ----------------------------------------------- */
static void _timer_func(void *para);

/* Private variables ---------------------------------------------------------*/
static const elab_dev_ops_t _button_ops =
{
    .enable = NULL,
    .read = NULL,
    .write = NULL,
};

static const osTimerAttr_t timer_attr_button =
{
    .name = "button_timer",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0,
};

static const osMutexAttr_t mutex_attr_button =
{
    .name = "mutex_button",
    .attr_bits = osMutexPrioInherit | osMutexRecursive,
    .cb_mem = NULL,
    .cb_size = 0, 
};

/* public function ---------------------------------------------------------- */
void elab_button_register(elab_button_t *const me, const char *name,
                            elab_button_ops_t *ops, void *user_data)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    elab_assert(ops != NULL);

    me->ops = ops;
    me->super.ops = &_button_ops;
    me->super.user_data = user_data;
    me->pressed = false;
    me->flag_dejitter = 0;
    me->time_pressed = 0;
    me->cb = NULL;
    for (uint8_t i = 0; i < ELAB_BUTTON_EVT_MAX; i ++)
    {
        me->e_sig[i] = 0;
    }

    me->timer = osTimerNew(_timer_func, osTimerPeriodic, me, &timer_attr_button);
    elab_assert(me->timer != NULL);
    
    me->mutex = osMutexNew(&mutex_attr_button);
    elab_assert(me->mutex != NULL);

    /* Register to device manager */
    elab_device_attr_t attr_button =
    {
        .name = name,
        .sole = false,
        .type = ELAB_DEVICE_UNKNOWN,
    };
    elab_device_register(&me->super, &attr_button);

    /* Start the button timer. */
    osStatus_t ret_os = osTimerStart(me->timer, 5);
    elab_assert(ret_os == osOK);
}

void elab_button_unregister(elab_button_t *const me)
{
    osStatus_t ret_os = osTimerDelete(me->timer);
    elab_assert(ret_os == osOK);

    elab_device_unregister(&me->super);
}

bool elab_button_is_pressed(elab_device_t *const me)
{
    elab_device_lock(me);
    bool status = ELAB_BUTTON_CAST(me)->pressed;
    elab_device_unlock(me);

    return status;
}

void elab_button_set_event_signal(elab_device_t *const me, uint8_t id, uint32_t e_sig)
{
    elab_assert(me != NULL);
    elab_assert(id < ELAB_BUTTON_EVT_MAX);

    elab_device_lock(me);
    ELAB_BUTTON_CAST(me)->e_sig[id] = e_sig;
    elab_device_unlock(me);
}

void elab_button_set_event_callback(elab_device_t *const me, elab_button_cb_t cb)
{
    elab_device_lock(me);
    ELAB_BUTTON_CAST(me)->cb = cb;
    elab_device_unlock(me);
}

void elab_button_clear_event_signal(elab_device_t *const me)
{
    elab_assert(me != NULL);

    elab_device_lock(me);
    for (uint32_t id = 0; id < ELAB_BUTTON_EVT_MAX; id ++)
    {
        ELAB_BUTTON_CAST(me)->e_sig[id] = 0;
    }
    elab_device_unlock(me);
}

/* private function --------------------------------------------------------- */
elab_inline void _event_publish(elab_button_t *const me, uint32_t event)
{
    if (me->e_sig[event] != 0)
    {
        QF_publish_(Q_NEW(QEvt, me->e_sig[event]));
    }

    if (me->cb != NULL)
    {
        me->cb(me, event);
    }
}

static void _timer_func(void *para)
{
    elab_button_t *me = (elab_button_t *)para;
    uint32_t time_diff = 0;

    elab_device_lock(me);

    /* De-jitter */
    bool pressed = me->ops->is_pressed(me);
    me->flag_dejitter <<= 1;
    if (pressed)
    {
        me->flag_dejitter |= 1;
    }
    if (!me->pressed && ((me->flag_dejitter & 0x0f) == 0x0f))
    {
        me->pressed = true;
        me->time_pressed = osKernelGetTickCount();
        _event_publish(me, ELAB_BUTTON_EVT_PRESSED);
    }
    else if (me->pressed && ((me->flag_dejitter & 0x0f) == 0x00))
    {
        me->pressed = false;
        me->time_release = osKernelGetTickCount();
        _event_publish(me, ELAB_BUTTON_EVT_RELEASE);
    }

    /* State machine. */
    switch (me->state)
    {
    case BUTTON_STATE_IDLE:
        if (me->pressed)
        {
            me->state = BUTTON_STATE_PRESSED;
        }
        break;

    case BUTTON_STATE_PRESSED:
        if (!me->pressed)
        {
            time_diff = osKernelGetTickCount() - me->time_pressed;
            if (time_diff >= ELAB_BUTTON_LONGPRESS_TIME_MIN)
            {
                me->state = BUTTON_STATE_IDLE;
                _event_publish(me, ELAB_BUTTON_EVT_LONGPRESS);
            }
            else if (time_diff >= ELAB_BUTTON_CLICK_TIME_MIN &&
                        time_diff <= ELAB_BUTTON_CLICK_TIME_MAX)
            {
                me->state = BUTTON_STATE_DOUBLE_CLICK_IDLE;
            }
        }
        break;

    case BUTTON_STATE_DOUBLE_CLICK_IDLE:
        if (me->pressed)
        {
            me->state = BUTTON_STATE_DOUBLE_CLICK_PRESSED;
        }
        if ((osKernelGetTickCount() - me->time_release)
            >= ELAB_BUTTON_DOUBLE_CLICK_IDLE_TIME_MAX)
        {
            _event_publish(me, ELAB_BUTTON_EVT_CLICK);
            me->state = BUTTON_STATE_IDLE;
        }
        break;

    case BUTTON_STATE_DOUBLE_CLICK_PRESSED:
        if (!me->pressed)
        {
            uint32_t time_diff = osKernelGetTickCount() - me->time_pressed;
            if (time_diff >= ELAB_BUTTON_CLICK_TIME_MIN &&
                time_diff <= ELAB_BUTTON_CLICK_TIME_MAX)
            {
                me->state = BUTTON_STATE_IDLE;
                _event_publish(me, ELAB_BUTTON_EVT_DOUBLE_CLICK);
            }
        }
        break;

    default:
        me->state = BUTTON_STATE_IDLE;
        break;
    }

    elab_device_unlock(me);
}

/* ----------------------------- end of file -------------------------------- */
