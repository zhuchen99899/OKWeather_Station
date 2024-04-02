/**
 * @file 74hc4052d.c
 * @author zc (387646983@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-01-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */


#ifndef __ELAB_74HC4052D_H_
#define __ELAB_74HC4052D_H_
#include "elab/edf/normal/elab_pin.h"


typedef enum 
{
CHANNEL_74HC4052D_NONE,
CHANNEL_74HC4052D_NY0,
CHANNEL_74HC4052D_NY1,
CHANNEL_74HC4052D_NY2,
CHANNEL_74HC4052D_NY3,
DISABLE_74HC4052D
}elab_74hc4052d_channel_t;

typedef struct elab_74hc4052d
{
elab_device_t super;        //设备层基类
elab_device_t *en_pin;      //使能引脚
elab_device_t *switch0_pin; //切换引脚0
elab_device_t *switch1_pin;     //切换引脚1
uint8_t channel_74hc4052d; //74hc4052d使能的通道
}elab_74hc4052d_t;


void ic_74hc4052d_register(elab_74hc4052d_t *me,const char *name, const char *en_pin_name,
                            const char * switch0_pin_name,const char * switch1_pin_name);

void ic_74hc4052d_switch(elab_device_t *me,elab_74hc4052d_channel_t swtich_type);
void ic_74hc405d_disable(elab_device_t *me);
#endif // !



