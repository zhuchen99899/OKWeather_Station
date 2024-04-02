/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include "elab_led.h"
#include "../normal/elab_pin.h"
#include "../../common/elab_assert.h"

ELAB_TAG("EdfLed");

/* private config ----------------------------------------------------------- */
#define ELAB_LED_POLL_PEROID                    (50)         //设置周期
#define ELAB_LED_ON_LONG_MS                     (2000)       //长亮灯时间
#define ELAB_LED_ON_SHORT_MS                    (500)        //短亮灯时间
#define ELAB_LED_OFF_MS                         (500)        //LED关闭时间

/* private defines ---------------------------------------------------------- */
enum elab_led_mode
{
    ELAB_LED_MODE_NULL = 0,
    ELAB_LED_MODE_TOGGLE,         //翻转模式
    ELAB_LED_MODE_VALUE,          //值模式
};


static void _timer_func(void *argument); //os定时器回调函数

/* Private variables ---------------------------------------------------------*/
static const elab_dev_ops_t _led_ops =
{
    .enable = NULL,
    .read = NULL,
    .write = NULL,
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
    .poll = NULL,
#endif
};                                        
 //os 定时器
static const osTimerAttr_t timer_attr_led =
{
    .name = "led_timer",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0,
};

/* public function ---------------------------------------------------------- */
//led注册  注册前pin设备需要首先注册 
void elab_led_register(elab_led_t *const me, const char *name,
                        const char *pin_name, bool status_led_on)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    elab_assert(pin_name != NULL);
    elab_assert(!elab_device_valid(name));
    elab_assert(elab_device_valid(pin_name));

    osStatus_t ret_os = osOK;
    
    /* Newly establish the timer and start it. */
    //创建系统定时器
    me->timer = osTimerNew(_timer_func, osTimerPeriodic, me, &timer_attr_led);
    assert_name(me->timer != NULL, name);
    ret_os = osTimerStart(me->timer, ELAB_LED_POLL_PEROID);
    elab_assert(ret_os == osOK);

    /* Set the data of the device. */
    me->super.ops = &_led_ops; //dev接口
    me->super.user_data = NULL; //抽象设备层无驱动数据
    me->mode = ELAB_LED_MODE_NULL;
    me->status = false;
    me->status_led_on = status_led_on; //初始状态
    me->pin = elab_device_find(pin_name);  //找到led对应的pin设备

    /* Register to device manager. */
    elab_device_attr_t attr_led =
    {
        .name = name,
        .sole = false,
        .type = ELAB_DEVICE_UNKNOWN,
    };
    elab_device_register(&me->super, &attr_led); //设备层注册
}

//led 设置状态 ELAB_LED_MODE_NULL模式
void elab_led_set_status(elab_device_t *const me, bool status)
{
    elab_assert(me != NULL);

    elab_led_t *led = ELAB_LED_CAST(me);

    led->mode = ELAB_LED_MODE_NULL;
}


/**
  * @brief  The led device toggle setting function.
  * @param  me      elab led device handle.
  * @param  period_ms   toggle period  
  * @retval None.
  * @note 周期翻转 ELAB_LED_MODE_TOGGLE模式
  */
void elab_led_toggle(elab_device_t *const me, uint32_t period_ms)
{
    elab_assert(me != NULL);
    elab_assert(period_ms >= ELAB_LED_POLL_PEROID);
    
    elab_led_t *led = ELAB_LED_CAST(me);

    if (led->mode != ELAB_LED_MODE_TOGGLE || led->period_ms != period_ms) //防止重复设置
    {
        led->mode = ELAB_LED_MODE_TOGGLE;  //模式
        led->time_out = osKernelGetTickCount() + period_ms; //溢出时间
        led->period_ms = period_ms;        //周期间隔
    }
}

/**
  * @brief  The led device value setting function.
  * @param  me      elab led device handle.
  * @param  value   The led device value.
  * @retval None. 
  * @note 长亮短亮模式 ELAB_LED_MODE_VALUE
  * 
  */
void elab_led_set_value(elab_device_t *const me, uint8_t value)
{
    elab_assert(me != NULL);

    elab_led_t *led = ELAB_LED_CAST(me);
    if (led->mode != ELAB_LED_MODE_VALUE || led->period_ms != value)
    {
        led->mode = ELAB_LED_MODE_VALUE; //模式
        led->value = value;              //设置值
        led->value_count = 0;            //计数
        led->value_count_max = (value + 1) * 2; //最大计数(value_count_max次后常亮) 
        led->time_out = osKernelGetTickCount() + ELAB_LED_ON_LONG_MS; //溢出时间 (最大)
        led->status = 0;                //led初始状态
        elab_pin_set_status(led->pin, !led->status_led_on);
    }
}

/* private function --------------------------------------------------------- */
/**
  * @brief  eLab LED polling timer function.
  * @param  argument    Timer function argument.
  * @retval None
  * @note 定时器回调
  */
static void _timer_func(void *argument)
{
    elab_led_t *led = ELAB_LED_CAST(argument);

    /* When led is working in TOGGLE mode. */
    if (led->mode == ELAB_LED_MODE_TOGGLE)
    {
        if (osKernelGetTickCount() >= led->time_out) //溢出
        {
            led->status = !led->status;  //翻转
            elab_pin_set_status(led->pin,
                                led->status_led_on ? led->status : !led->status); //设置底层pin
            led->time_out += led->period_ms; //下一个溢出时间
        }
    }
    /* When led is working in VALUE mode. */
    else if (led->mode == ELAB_LED_MODE_VALUE)
    {
        if (osKernelGetTickCount() >= led->time_out)
        {
            led->status = !led->status; //翻转
            elab_pin_set_status(led->pin,
                                led->status_led_on ? led->status : !led->status);
            led->value_count ++; //计数
            if (led->value_count >= led->value_count_max) //LED长亮
            {
                led->value_count = 0;
            }

            if (led->value_count == 0) 
            {
                led->time_out += ELAB_LED_ON_LONG_MS;  //LED长亮时间
            }
            else if (led->value_count % 2 == 0)        //LED短亮时间
            {
                led->time_out += ELAB_LED_ON_SHORT_MS;
            }
            else                                        //LED关闭时间
            {
                led->time_out += ELAB_LED_OFF_MS;     
            }
        }
    }
}

/* ----------------------------- end of file -------------------------------- */
