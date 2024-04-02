/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_device.h"
#include "../os/cmsis_os.h"
#include "../common/elab_assert.h"
#include "../common/elab_export.h"

/* Private function prototypes -----------------------------------------------*/
static elab_err_t _null_enable(elab_device_t *me, bool status);
static int32_t _null_read(elab_device_t *me,
                            uint32_t pos, void *buffer, uint32_t size);
static int32_t _null_write(elab_device_t *me,
                            uint32_t pos, const void *buffer, uint32_t size);
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
static void _null_poll(elab_device_t *me);
#endif

/* Private variables ---------------------------------------------------------*/
/**
  * @brief  Firmware device's driver interface.
  */
static const elab_dev_ops_t dev_null_ops =
{
    .enable = _null_enable,
    .read = _null_read,
    .write = _null_write,
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
    .poll = _null_poll,
#endif
};

static elab_device_t dev_uart_null =
{
    .ops = &dev_null_ops,
};

static elab_device_attr_t dev_attr =
{
    .sole = false,
};
/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Null uart device initialization.
  */
static void uart_null_init(void)
{
    dev_attr.name = "SerialNull";
    dev_attr.type = ELAB_DEVICE_UART;
    elab_device_register(&dev_uart_null, &dev_attr);
}

/**
  * @brief  Export null uart device initialization function.
  */
INIT_EXPORT(uart_null_init, 0);

/* Private functions ---------------------------------------------------------*/
/**
 * The null function for the device interfaces: init, open and close.
 */
static elab_err_t _null_enable(elab_device_t *me, bool status)
{
    (void)me;
    (void)status;

    return ELAB_OK;
}

/**
 * The null function for the device read interface.
 */
static int32_t _null_read(elab_device_t *me,
                            uint32_t pos, void *buffer, uint32_t size)
{
    (void)me;
    (void)pos;
    (void)buffer;
    (void)size;

    return 0;
}

/**
 * The null function for the device write interface.
 */
static int32_t _null_write(elab_device_t *me,
                            uint32_t pos, const void *buffer, uint32_t size)
{
    (void)me;
    (void)pos;
    (void)buffer;

    return size;
}

#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
/**
 * The null function for the device poll interface.
 */
static void _null_poll(elab_device_t *me)
{
    (void)me;
}
#endif

/* ----------------------------- end of file -------------------------------- */
