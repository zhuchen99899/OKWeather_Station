#ifndef __ANEMOMETER_H_
#define __ANEMOMETER_H_

#include "elab_ms1022.h"
#include "elab_74hc4052d.h"


typedef struct elab_anemometer
{
elab_device_t super;        //设备层基类
elab_device_t *ultrasonic;   //超声波基类
// elab_device_t *ms1022;      //基于ms1022飞行时间芯片方案
// elab_device_t *fire_74hc4052; //fire切换
// elab_device_t *receive_74hc4052; //接收切换
osTimerId_t timer;
uint32_t period_ms;
uint32_t time_out;
uint8_t mode;
// uint8_t channel;

}elab_anemometer_t;

#define ELAB_ANEMOMETER_CAST(_dev)           ((elab_anemometer_t *)_dev)

typedef enum 
{
CHANNEL_ANEMOMETER_NONE,
CHANNEL_ANEMOMETER_NORTH_TO_SOUTH,
CHANNEL_ANEMOMETER_SOUTH_TO_NORTH,
CHANNEL_ANEMOMETER_EAST_TO_WEST,
CHANNEL_ANEMOMETER_WEST_TO_EAST,
CHANNEL_ANEMOMETER_TEST,
CHANNEL_DISABLE_ANEMOMETER
}elab_anemometer_channel_t;

enum elab_led_mode
{
    ELAB_ANEMOMETER_NULL = 0,
    ELAB_ANEMOMETER_PERIOD,         //风速风向仪周期计算
    ELAB_ANEMOMETER_STOP,          //停止
};


/*public_function*/
void elab_anemometer_register(elab_anemometer_t *me,const char *name,
                                const char *ultrasonic_name);
void elab_anemometer_period(elab_device_t *const me, uint32_t period_ms);
void elab_anemometer_stop(elab_device_t *const me);
#endif // !__ANEMOMETER_H_
