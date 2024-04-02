
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab/common/elab_export.h"
#include "../../common/elab_assert.h"
#include "elab/edf/user/elab_led.h"
#include "export.h"

/* private variables -------------------------------------------------------- */
static elab_led_t led1;
#define LED_TOGGLE_PERIOD_MS                  (500)      //500MS闪烁

ELAB_TAG("LED_EXPORT");
/* public functions --------------------------------------------------------- */
static void led_export(void)
{
    /* PIN devices on MCU. */
            // led抽象设备  led名称    引脚名称    初始状态
    elab_led_register(&led1, "led1", "pin_led1", true);
    elab_assert(&led1 != NULL);
	elab_led_toggle(&led1.super,LED_TOGGLE_PERIOD_MS);
}
INIT_EXPORT(led_export, EXPORT_LEVEL_LED);

/* ----------------------------- end of file -------------------------------- */
