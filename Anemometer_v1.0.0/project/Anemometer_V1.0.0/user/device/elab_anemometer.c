/**
 * @file elab_anemometer.c
 * @author zc (387646983@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-01-23
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "elab_anemometer.h"
#include "../../common/elab_assert.h"

#ifdef __cplusplus
extern "C" {
#endif


ELAB_TAG("Edf_anemometer");
#define ELAB_ANEMOMETER_POLL_PEROID                    (50)         //设置定时器回调执行周期

/* Private variables ---------------------------------------------------------*/
static const elab_dev_ops_t anemometer_ops =
{
   .enable = NULL,
   .read = NULL,
   .write = NULL,
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
   .poll = NULL,
#endif
};

 //os 定时器
static const osTimerAttr_t timer_attr_anemometer =
{
    .name = "anemometer_timer",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0,
};
static void _timer_func(void *argument); //os定时器回调函数

/**
 * @brief 风速风向仪设备层注册
 * 
 * @param me  设备对象
 * @param name 名称
 * @param ms1022_name  TOF芯片名称
 * @param fire_name  fire切换
 * @param receive_name  接收切换
 */
void elab_anemometer_register(elab_anemometer_t *me,const char *name,
                                const char *ultrasonic_name)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    elab_assert(ultrasonic_name != NULL);


    elab_assert(!elab_device_valid(name));
    elab_assert(elab_device_valid(ultrasonic_name));


    osStatus_t ret_os = osOK;
    me->ultrasonic =elab_device_find(ultrasonic_name);


    //poll定时器
    me->timer = osTimerNew(_timer_func, osTimerPeriodic, me, &timer_attr_anemometer);
    assert_name(me->timer != NULL, name);
    ret_os = osTimerStart(me->timer, ELAB_ANEMOMETER_POLL_PEROID);
    elab_assert(ret_os == osOK);

    //device注册
      elab_device_attr_t attr_anemometer =
   {
       .name = name,
       .sole = true,
       .type = ELAB_DEVICE_UNKNOWN,
   }; //设备属性 

    me->super.ops=&anemometer_ops;

    elab_device_register(&me->super, &attr_anemometer);//设备层注册
}



/**
 * @brief 设置超声波方向
 * @param me 
 * @param channel  方向 枚举
 * @note 自动使能 enable
 */
// static void elab_anemometer_Set_Channel(elab_anemometer_t *anemometer,elab_anemometer_channel_t channel)
// {

//     assert(anemometer != NULL);

//     switch (channel)
//     {
//     case CHANNEL_ANEMOMETER_NORTH_TO_SOUTH: //北->南
//     ic_74hc4052d_switch(anemometer->fire_74hc4052,CHANNEL_74HC4052D_NY1);
//     ic_74hc4052d_switch(anemometer->receive_74hc4052,CHANNEL_74HC4052D_NY3);
//     anemometer->channel=CHANNEL_ANEMOMETER_NORTH_TO_SOUTH;
//         break;
//     case CHANNEL_ANEMOMETER_SOUTH_TO_NORTH://南->北
//     ic_74hc4052d_switch(anemometer->fire_74hc4052,CHANNEL_74HC4052D_NY3);
//     ic_74hc4052d_switch(anemometer->receive_74hc4052,CHANNEL_74HC4052D_NY1);
//     anemometer->channel=CHANNEL_ANEMOMETER_SOUTH_TO_NORTH;
//         break;
        
//     case CHANNEL_ANEMOMETER_EAST_TO_WEST://东->西
//     ic_74hc4052d_switch(anemometer->fire_74hc4052,CHANNEL_74HC4052D_NY0);
//     ic_74hc4052d_switch(anemometer->receive_74hc4052,CHANNEL_74HC4052D_NY2);
//     anemometer->channel=CHANNEL_ANEMOMETER_EAST_TO_WEST;
//         break;   

//     case CHANNEL_ANEMOMETER_WEST_TO_EAST://西->东
//     ic_74hc4052d_switch(anemometer->fire_74hc4052,CHANNEL_74HC4052D_NY2);
//     ic_74hc4052d_switch(anemometer->receive_74hc4052,CHANNEL_74HC4052D_NY0);
//     anemometer->channel=CHANNEL_ANEMOMETER_WEST_TO_EAST;
//         break;

//     case CHANNEL_DISABLE_ANEMOMETER: //不设置方向
//     ic_74hc405d_disable(anemometer->fire_74hc4052);
//     ic_74hc405d_disable(anemometer->receive_74hc4052);
//     anemometer->channel=CHANNEL_DISABLE_ANEMOMETER;
//         break;

//     case CHANNEL_ANEMOMETER_TEST: //同一超声波探头
//     ic_74hc4052d_switch(anemometer->fire_74hc4052,CHANNEL_74HC4052D_NY1);
//     ic_74hc4052d_switch(anemometer->receive_74hc4052,CHANNEL_74HC4052D_NY1);
//     anemometer->channel=CHANNEL_DISABLE_ANEMOMETER;
//         break;

//     default:

//         break;
//     }
// }

/**
 * @brief 获取一次TOF
 * 
 * @param me 
 */
// static void elab_anemometer_get_TOF(elab_anemometer_t *anemometer,elab_anemometer_channel_t channel)
// {

    // assert(anemometer != NULL);
    // float data;
    // elab_anemometer_Set_Channel(anemometer,channel);
    
    // elab_ms1022_send_cmd(anemometer->ms1022,MS1022CMD_Init);
    // elab_ms1022_send_cmd(anemometer->ms1022,MS1022CMD_Start_TOF);
    // elab_ms1022_wait_int(anemometer->ms1022);

    // elab_ms1022_read_error_bit(anemometer->ms1022);
    // data=elab_ms1022_read_TOF(anemometer->ms1022);
    // elog_debug("data=%f",data);
    
// }




/**
 * @brief 计算风速
 * 
 * @param me 
 */
static void elab_anemometer_calculate_wind_speed(elab_anemometer_t *anemometer)
{
    
    assert(anemometer != NULL);
    float data= elab_ultransonic_get_fly_time(anemometer->ultrasonic);      
    // elab_ms1022_freq_corr_fact(anemometer->ms1022);
    // elab_anemometer_get_TOF(anemometer,channel);
    elog_debug("data%f",data);
}



//public functions

/**
 * @brief 设置anenmometer周期性计算模式
 * 
 * @param me 
 * @param period_ms  周期
 */
void elab_anemometer_period(elab_device_t *const me, uint32_t period_ms)
{
    elab_assert(me != NULL);
    elab_assert(period_ms >= ELAB_ANEMOMETER_POLL_PEROID);
    
    elab_anemometer_t *anemometer = ELAB_ANEMOMETER_CAST(me);

    if (anemometer->mode != ELAB_ANEMOMETER_PERIOD ||anemometer->period_ms != period_ms) //防止重复设置
    {
        anemometer->mode = ELAB_ANEMOMETER_PERIOD;  //模式
        anemometer->time_out = osKernelGetTickCount() + period_ms; //溢出时间
        anemometer->period_ms = period_ms;        //周期间隔
    }
}


/**
 * @brief 设置elab_anemometer 停止
 * 
 * @param me 
 */
void elab_anemometer_stop(elab_device_t *const me)
{
    elab_assert(me != NULL);
    elab_anemometer_t *anemometer = ELAB_ANEMOMETER_CAST(me);

 if (anemometer->mode != ELAB_ANEMOMETER_STOP)
 {
    anemometer->mode = ELAB_ANEMOMETER_STOP;  //模式
 }
    
}

/**
 * @brief 定时器回调
 * 
 * @param argument 
 */
static void _timer_func(void *argument)
{
    elab_anemometer_t *anemometer = ELAB_ANEMOMETER_CAST(argument);
    if (anemometer->mode == ELAB_ANEMOMETER_PERIOD)
    {
        if (osKernelGetTickCount() >= anemometer->time_out) //溢出
        {
            elab_anemometer_calculate_wind_speed(anemometer);
            anemometer->time_out += anemometer->period_ms; //下一个溢出时间
        }
    }
    else if(anemometer->mode == ELAB_ANEMOMETER_STOP)
    {

    //null

    }
}



#ifdef __cplusplus
}
#endif


