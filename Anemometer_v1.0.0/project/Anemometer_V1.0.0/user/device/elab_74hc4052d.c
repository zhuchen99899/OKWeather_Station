/**
 * @file elab_74hc4052d.c
 * @author zc (387646983@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-01-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "elab_74hc4052d.h"
#include "../../common/elab_assert.h"
#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("Edf_74hc4052d");
/* Private variables ---------------------------------------------------------*/
static const elab_dev_ops_t ic_74hc4052d_ops =
{
   .enable = NULL,
   .read = NULL,
   .write = NULL,
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
   .poll = NULL,
#endif
};


/**
 * @brief 74hc4052d芯片注册
 * 
 * @param me  elab_74hc4052d_t 对象
 * @param name  74hc4052d名称
 * @param en_pin_name  使能引脚名称
 * @param switch0_pin_name 切换引脚名称1
 * @param switch1_pin_name 切换引脚名称2
 */
void ic_74hc4052d_register(elab_74hc4052d_t *me,const char *name, const char *en_pin_name,
                            const char * switch0_pin_name,const char * switch1_pin_name)
{
    elab_assert(me != NULL);
    elab_assert(name != NULL);
    elab_assert(en_pin_name != NULL);
    elab_assert(switch0_pin_name != NULL);
    elab_assert(switch1_pin_name != NULL);
    elab_assert(!elab_device_valid(name));
    elab_assert(elab_device_valid(en_pin_name));
    elab_assert(elab_device_valid(switch0_pin_name));
    elab_assert(elab_device_valid(switch1_pin_name));

    me->en_pin=elab_device_find(en_pin_name);
    me->switch0_pin=elab_device_find(switch0_pin_name);
    me->switch1_pin=elab_device_find(switch1_pin_name);
    me->channel_74hc4052d=CHANNEL_74HC4052D_NONE;
    elab_device_attr_t attr_ic_74hc4052d =
   {
       .name = name,
       .sole = true,
       .type = ELAB_DEVICE_UNKNOWN,
   }; //设备属性 

    me->super.ops=&ic_74hc4052d_ops;
    elab_device_register(&me->super, &attr_ic_74hc4052d);//设备层注册
}


/**
 * @brief 74hc4052d切换通道
 * 
 * @param me 
 * @param swtich_type 切换的通道
 * @note 切换通道时74hc4052d芯片使能
 */
void ic_74hc4052d_switch(elab_device_t *me,elab_74hc4052d_channel_t swtich_type)
{
    assert(me != NULL);
    elab_74hc4052d_t *ic_74hc4052d=(elab_74hc4052d_t*) me;
    elab_pin_set_status(ic_74hc4052d->en_pin,false);//低电平使能
    switch (swtich_type)
    {
    case CHANNEL_74HC4052D_NY0:
    elab_pin_set_status(ic_74hc4052d->switch0_pin,false);
    elab_pin_set_status(ic_74hc4052d->switch1_pin,false);
    ic_74hc4052d->channel_74hc4052d=CHANNEL_74HC4052D_NY0;
        break;

    case CHANNEL_74HC4052D_NY1:
    elab_pin_set_status(ic_74hc4052d->switch0_pin,true);
    elab_pin_set_status(ic_74hc4052d->switch1_pin,false);
    ic_74hc4052d->channel_74hc4052d=CHANNEL_74HC4052D_NY1;
        break;

    case CHANNEL_74HC4052D_NY2:
    elab_pin_set_status(ic_74hc4052d->switch0_pin,false);
    elab_pin_set_status(ic_74hc4052d->switch1_pin,true);
    ic_74hc4052d->channel_74hc4052d=CHANNEL_74HC4052D_NY2;
        break;

    case CHANNEL_74HC4052D_NY3:
    elab_pin_set_status(ic_74hc4052d->switch0_pin,true);
    elab_pin_set_status(ic_74hc4052d->switch1_pin,true);
    ic_74hc4052d->channel_74hc4052d=CHANNEL_74HC4052D_NY3;
        break;
    default:
        break;
    }

}

/**
 * @brief 74hc405d芯片关闭
 * 
 * @param me 
 */
void ic_74hc405d_disable(elab_device_t *me)
{
   assert(me != NULL);
   elab_74hc4052d_t *ic_74hc4052d=(elab_74hc4052d_t*) me;
   elab_pin_set_status(ic_74hc4052d->en_pin,true);
    ic_74hc4052d->channel_74hc4052d=DISABLE_74HC4052D;
}

#ifdef __cplusplus
}
#endif

