/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef __ELAB_CAN_H__
#define __ELAB_CAN_H__

/* include ------------------------------------------------------------------ */
#include "../elab_device.h"
#include "../../os/cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif

/* public define ------------------------------------------------------------ */
enum
{
    ELAB_CAN_BAUDRATE_1M = 0,
    ELAB_CAN_BAUDRATE_500K,
    ELAB_CAN_BAUDRATE_250K,
    ELAB_CAN_BAUDRATE_125K,
    ELAB_CAN_BAUDRATE_100K,

    ELAB_CAN_BAUDRATE_MAX
};

enum
{
    ELAB_CAN_MODE_NORMAL = 0,
    ELAB_CAN_MODE_LOOPBACK,
};

enum
{
    ELAB_CAN_FILTER_MODE_MASK = 0,
    ELAB_CAN_FILTER_MODE_LIST,
};

/* public typedef ----------------------------------------------------------- */
typedef struct elab_can_config
{
    uint8_t baud_rate;
    uint8_t mode;
} elab_can_config_t;

typedef struct elab_can_msg
{
    uint32_t id             : 29;               /* CAN ID */
    uint32_t ide            : 1;                /* Extend CAN ID */
    uint32_t rtr            : 1;                /* Remote CAN Message */
    uint32_t reserve        : 1;
    uint8_t data[8];
    uint8_t length;
} elab_can_msg_t;

typedef struct elab_can_attribute
{
    void *user_data;
    uint16_t buff_size_send;
    uint16_t buff_size_recv;
} elab_can_attr_t;

typedef struct elab_can_buff
{
    elab_can_msg_t *msg;
    uint8_t size;
    uint8_t head;
    uint8_t tail;
} elab_can_buff_t;

typedef struct elab_can_filter
{
    struct elab_can_filter *next;
    uint32_t id             : 29;
    uint32_t mask           : 29;
    uint32_t mode           : 1;
    uint32_t ide            : 1;
    uint32_t rtr            : 1;
} elab_can_filter_t;

typedef struct elab_can
{
    elab_device_t super;

    const struct elab_can_ops * ops;
#if !defined(__linux__) && !defined(_WIN32)
    osMessageQueueId_t mq_recv;
#endif
    elab_can_config_t config;
    elab_can_filter_t *filter_list;
    bool sending;
} elab_can_t;

struct elab_can_ops
{
#if !defined(__linux__) && !defined(_WIN32)
    bool (* send_busy)(elab_can_t * const me);
#endif

    elab_err_t (* enable)(elab_can_t * const me, bool en_status);
    elab_err_t (* config)(elab_can_t * const me, elab_can_config_t * config);
    void (* send)(elab_can_t * const me, const elab_can_msg_t *msg);
#if defined(__linux__) || defined(_WIN32)
    int32_t (* recv)(elab_can_t * const me, elab_can_msg_t *msg);
#endif
    elab_err_t (* config_filter)(elab_can_t * const me, elab_can_filter_t *filter);
};

/* public function ---------------------------------------------------------- */
/* For the driver layer. */
void elab_can_register(elab_can_t * const me, const char * name,
                        elab_can_attr_t *attr, void *user_data);
#if !defined(__linux__) && !defined(_WIN32)
void elab_can_isr_recv(elab_can_t * const me, elab_can_msg_t *msg);
void elab_can_isr_send_end(elab_can_t * const me);
#endif

/* For the upper layers. */
void elab_can_config(elab_device_t * const me, elab_can_config_t *config);
void elab_can_send(elab_device_t * const me, const elab_can_msg_t *msg);
int32_t elab_can_recv(elab_device_t * const me, elab_can_msg_t *msg);
elab_err_t elab_can_config_filter(elab_device_t * const me, elab_can_filter_t *filter);

#ifdef __cplusplus
}
#endif

#endif /* __Elab_CAN_H__ */

/* ----------------------------- end of file -------------------------------- */
