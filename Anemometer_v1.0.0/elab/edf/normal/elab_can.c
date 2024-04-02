/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include <string.h>
#include "elab_can.h"
#include "../../common/elab_assert.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("Edf_CAN");

/* private function prototype ----------------------------------------------- */
/* Interface functions. */
static elab_err_t _can_enable(elab_device_t * const me, bool status);

/* private variables -------------------------------------------------------- */
static const elab_dev_ops_t can_ops =
{
    _can_enable,
    NULL,
    NULL,
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
    NULL,
#endif
};

/* public function ---------------------------------------------------------- */
void elab_can_register(elab_can_t * const me, const char * name,
                        elab_can_attr_t *attr, void *user_data)
{
    /* Check the parameters are valid or not. */
    elab_assert(name != NULL);
    assert_name(me != NULL, name);
    assert_name(attr != NULL, name);
    assert_name(me->ops != NULL, name);
    assert_name(attr->buff_size_send != 0, name);
    assert_name(attr->buff_size_recv != 0, name);

    /* The message queue initializations. */
#if !defined(__linux__) && !defined(_WIN32)
    me->mq_recv = osMessageQueueNew(attr->buff_size_recv,
                                    sizeof(elab_can_msg_t), NULL);
    elab_assert(me->mq_recv != NULL);
#endif

    /* Configure CAN bus as default and disable the ISR. */
    const elab_can_config_t config_default =
    {
        (uint8_t)ELAB_CAN_BAUDRATE_125K,
        (uint8_t)ELAB_CAN_MODE_NORMAL,
    };
    me->ops->config(me, (elab_can_config_t *)&config_default);
    me->config = config_default;
    me->ops->enable(me, false);

    /* Set the ops of the super class. */
    me->super.ops = (elab_dev_ops_t *)&can_ops;

    /* Register the CAN bus to the EIO framwork. */
    elab_device_attr_t can_device_attr =
    {
        name, false, ELAB_DEVICE_CAN,
    };
    elab_device_register(&me->super, &can_device_attr);
    me->super.user_data = user_data;
}

#if !defined(__linux__) && !defined(_WIN32)
void elab_can_isr_recv(elab_can_t * const me, elab_can_msg_t *msg)
{
    /* Check the parameters are valid or not. */
    elab_assert(me != NULL);
    assert_name(msg != NULL, me->super.name);
    assert_name(me->ops != NULL, me->super.name);
    assert_name(me->ops->isr_enable != NULL, me->super.name);

    /* Check the filter. */
    bool valid = false;
    if (me->ops->config_filter == NULL)         /* If soft filter. */
    {
        elab_can_filter_t *filter = me->filter_list;
        while (filter != NULL)
        {
            if ((filter->id & filter->mask) == (msg->id & filter->mask))
            {
                valid = true;
                break;
            }
            filter = filter->next;
        }
    }

    if (valid)
    {
        /* Push the CAN message into buffer. */
        eio_can_buff_t *buff = &me->buff_receive;
        me->ops->isr_enable(me, false);
        assert_name(_buff_remaining(buff) > 0, me->super.name);
        buff->msg[buff->head] = *msg;
        buff->head = (buff->head + 1) % buff->size;
        me->ops->isr_enable(me, true);

        /* Send the event of message received. */
        eio_event_publish(&me->super, EIO_CAN_EVT_MESSAGE_RECEIVED);
    }
}

void elab_can_isr_send_end(elab_can_t * const me)
{
    /* Check the parameters are valid or not. */
    elab_assert(me != NULL);
    assert_name(me->ops != NULL);
    assert_name(me->ops->isr_enable != NULL);
    assert_name(me->ops->send != NULL);

    while (1)
    {
        eio_can_buff_t *buff = &me->buff_send;
        if (_buff_empty(buff))
        {
            me->sending = false;
            break;
        }

        /* Get one message from the sending buffer and send it. */
        me->ops->send(me, &buff->msg[buff->tail]);
        buff->tail = (buff->tail + 1) % buff->size;

        /* Check the CAN bus is busy. */
        if (me->ops->send_busy == NULL)
        {
            break;
        }
        if (me->ops->send_busy(me))
        {
            break;
        }
    }
}
#endif

void elab_can_config(elab_device_t * const me, elab_can_config_t *config)
{
    /* Check the parameters are valid or not. */
    elab_assert(me != NULL);
    assert_name(config != NULL, me->attr.name);

    /* CAN type cast. */
    elab_can_t *can = (elab_can_t *)me;
    assert_name(can->ops != NULL, me->attr.name);
    assert_name(can->ops->config != NULL, me->attr.name);

    /* If configurations of CAN bus are changed, */
    if (memcmp(config, &can->config, sizeof(elab_can_config_t)) != 0)
    {
        /* Configure the CAN bus. */
        can->ops->config(can, config);
        memcpy(&can->config, config, sizeof(elab_can_config_t));
    }
}

void elab_can_send(elab_device_t * const me, const elab_can_msg_t *msg)
{
    int32_t ret = 0;
    
    /* Check the parameters are valid or not. */
    assert_name(me != NULL, me->attr.name);
    assert_name(msg != NULL, me->attr.name);

    /* CAN type cast. */
    elab_can_t *can = (elab_can_t *)me;
    assert_name(can->ops != NULL, me->attr.name);

#if !defined(__linux__) && !defined(_WIN32)

#else
    can->ops->send(can, msg);
#endif
}

int32_t elab_can_recv(elab_device_t * const me, elab_can_msg_t *msg)
{
    int32_t ret = 0;
    
    /* Check the parameters are valid or not. */
    assert_name(me != NULL, me->attr.name);
    assert_name(msg != NULL, me->attr.name);

    /* CAN type cast. */
    elab_can_t *can = (elab_can_t *)me;
    assert_name(can->ops != NULL, me->attr.name);

#if !defined(__linux__) && !defined(_WIN32)
    osStatus_t ret_os = osOK;
    ret_os = osMessageQueueGet(&can->mq_recv, msg, NULL, osWaitForever);
    elab_assert(ret_os == osOK);
    ret = 1;
#else
    ret = can->ops->recv(can, msg);
#endif

    return ret;
}

elab_err_t eio_can_config_filter(elab_device_t * const me, elab_can_filter_t *filter)
{
    elab_err_t ret = ELAB_OK;

    /* Check the parameters are valid or not. */
    elab_assert(me != NULL);
    elab_assert(filter != NULL);

    /* CAN type cast. */
    elab_can_t *can = (elab_can_t *)me;
    assert_name(can->ops != NULL, me->attr.name);

    /* Set the hardware CAN filter. */
    if (can->ops->config_filter != NULL)
    {
        ret = can->ops->config_filter(can, filter);
    }

    /* Add to the list. */
    if (ret == ELAB_OK)
    {
        filter->next = can->filter_list;
        can->filter_list = filter;
    }

    return ret;
}

/* private function --------------------------------------------------------- */
static elab_err_t _can_enable(elab_device_t * const me, bool status)
{
    /* can type cast. */
    elab_can_t *can = (elab_can_t *)me;
    assert_name(can->ops != NULL, me->attr.name);
    assert_name(can->ops->enable != NULL, me->attr.name);

    /* Enable the CAN device driver. */
    can->ops->enable(can, status);

    return ELAB_OK;
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
