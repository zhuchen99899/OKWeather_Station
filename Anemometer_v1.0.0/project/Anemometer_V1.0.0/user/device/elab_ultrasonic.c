/**
 * @file elab_ultrasonic.c
 * @author zc (387646983@qq.com)
 * @brief 
 * @version 1.0.0
 * @date 2024-04-02
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "elab_ultrasonic.h"
#include "../../common/elab_assert.h"

ELAB_TAG("Edf_Ultrasonic");

/* Private variables ---------------------------------------------------------*/
static const elab_dev_ops_t ultrasonic_ops =
{
   .enable = NULL,
   .read = NULL,
   .write = NULL,
#if (ELAB_DEV_PALTFORM == ELAB_PALTFORM_POLL)
   .poll = NULL,
#endif
};

/**
 * @brief 超声波设备注册
 * 
 * @param me 超神波设备类
 * @param name  超声波设备名
 * @param ms1022_name  ms1022 设备名
 */
void elab_ultrasonic_register(elab_ultrasonic_t *me,const char *name,
                                const char *ms1022_name)
{

    elab_assert(me != NULL);
    elab_assert(name != NULL);
    elab_assert(ms1022_name != NULL);

    elab_assert(!elab_device_valid(name));
    elab_assert(elab_device_valid(ms1022_name));

    me->ms1022 = elab_device_find(ms1022_name);

    //device注册
      elab_device_attr_t attr_ultrasonic =
   {
       .name = name,
       .sole = true,
       .type = ELAB_DEVICE_UNKNOWN,
   }; //设备属性 

    me->super.ops=&ultrasonic_ops;

    elab_device_register(&me->super, &attr_ultrasonic);//设备层注册
}


float elab_ultransonic_get_fly_time(elab_device_t *me)
{
    elab_ultrasonic_t *ultransonic=(elab_ultrasonic_t*)me;
    assert(me != NULL);
    float data=caculate_tof(ultransonic->ms1022);
    return data;
}