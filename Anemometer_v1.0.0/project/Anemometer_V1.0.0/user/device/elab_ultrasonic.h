#ifndef __ELAB_ULTRASONIC_H_
#define __ELAB_ULTRASONIC_H_
#include "elab_ms1022.h"
typedef struct elab_ultrasonic
{
elab_device_t super;         //设备层基类
elab_device_t *ms1022;       //ms1022设备
}elab_ultrasonic_t;

void elab_ultrasonic_register(elab_ultrasonic_t *me,const char *name,
                                const char *ms1022_name);
float elab_ultransonic_get_fly_time(elab_device_t *me);                                
#endif
