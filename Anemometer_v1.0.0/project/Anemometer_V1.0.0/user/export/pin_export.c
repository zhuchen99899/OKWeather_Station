
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "../driver/drv_pin.h"
#include "./device/elab_74hc4052d.h"
#include "elab/common/elab_export.h"
#include "export.h"

/* private variables -------------------------------------------------------- */
static elab_pin_driver_t pin_mcu_led2_b03;
static elab_pin_driver_t pin_mcu_enfire_b0;
static elab_pin_driver_t pin_mcu_fire_switch0_b02;
static elab_pin_driver_t pin_mcu_fire_switch1_b10;
static elab_pin_driver_t pin_mcu_enreceive_c0;
static elab_pin_driver_t pin_mcu_receive_switch0_c02;
static elab_pin_driver_t pin_mcu_receive_switch1_c03;
static elab_pin_driver_t pin_mcu_cs_ms1022_a04;
static elab_pin_driver_t pin_mcu_INTN_ms1022_a05;

static elab_74hc4052d_t fire_74hc4052d; 
static elab_74hc4052d_t ut_receive_74hc4052d;
/* public functions --------------------------------------------------------- */
static void driver_pin_mcu_export(void)
{
    /* PIN devices on MCU. */
                        // 引脚抽象设备       引脚名称     引脚
    elab_driver_pin_init(&pin_mcu_led2_b03, "pin_led1", "B.03");
    elab_pin_set_mode(&pin_mcu_led2_b03.device.super, PIN_MODE_OUTPUT_PP);
    
    elab_driver_pin_init(&pin_mcu_enfire_b0, "en_fire", "B.00");
    elab_pin_set_mode(&pin_mcu_enfire_b0.device.super, PIN_MODE_OUTPUT_PP);

    elab_driver_pin_init(&pin_mcu_fire_switch0_b02, "fire_switch0", "B.02");
    elab_pin_set_mode(&pin_mcu_fire_switch0_b02.device.super, PIN_MODE_OUTPUT_PP);

    elab_driver_pin_init(&pin_mcu_fire_switch1_b10, "fire_switch1", "B.10");
    elab_pin_set_mode(&pin_mcu_fire_switch1_b10.device.super, PIN_MODE_OUTPUT_PP);

    elab_driver_pin_init(&pin_mcu_enreceive_c0, "en_receive", "C.00");
    elab_pin_set_mode(&pin_mcu_enreceive_c0.device.super, PIN_MODE_OUTPUT_PP);

    elab_driver_pin_init(&pin_mcu_receive_switch0_c02, "receive_switch0", "C.02");
    elab_pin_set_mode(&pin_mcu_receive_switch0_c02.device.super, PIN_MODE_OUTPUT_PP);

    elab_driver_pin_init(&pin_mcu_receive_switch1_c03, "receive_switch1", "C.03");
    elab_pin_set_mode(&pin_mcu_receive_switch1_c03.device.super, PIN_MODE_OUTPUT_PP);

    elab_driver_pin_init(&pin_mcu_cs_ms1022_a04,"pin_ms1022_cs","A.04");
    elab_pin_set_mode(&pin_mcu_cs_ms1022_a04.device.super, PIN_MODE_OUTPUT_PP);
    elab_pin_set_status(&pin_mcu_cs_ms1022_a04.device.super,true);

    elab_driver_pin_init(&pin_mcu_INTN_ms1022_a05,"pin_ms1022_intn","A.05");
     elab_pin_set_mode(&pin_mcu_INTN_ms1022_a05.device.super, PIN_MODE_INPUT_PULLDOWN);
}
INIT_EXPORT(driver_pin_mcu_export, EXPORT_LEVEL_PIN_MCU);


static void ic_74hc4052d_export(void)
{
    
ic_74hc4052d_register(&fire_74hc4052d,"fire_74hc4052d","en_fire","fire_switch0","fire_switch1");
ic_74hc4052d_register(&ut_receive_74hc4052d,"ut_receive_74hc4052d","en_receive","receive_switch0","receive_switch1");
}
INIT_EXPORT(ic_74hc4052d_export, EXPORT_LEVEL_USR_74HC405RD);

/* ----------------------------- end of file -------------------------------- */
