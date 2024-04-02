
/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include <string.h>
#include "elab_pin.h"
#include "../../common/elab_assert.h"

ELAB_TAG("Edf_Pin");

#ifdef __cplusplus
extern "C" {
#endif

/* public functions --------------------------------------------------------- */
/**
  * @brief  eLab pin initialization.
  * @param  me      this pointer
  * @param  name    pin's name.
  * @param  ops     抽象设备层pin的接口，底层驱动提供接口，应用层调用
  * @param  user_data 用户数据，由驱动注册，提供应用层获取驱动数据
  * @retval None    elab_pin抽象设备层注册函数 由底层驱动调用  
  */
void elab_pin_register(elab_pin_t * const me,
                        const char *name,
                        const elab_pin_ops_t *ops,
                        void *user_data)
{
    /*检查参数*/
    assert(me != NULL);
    assert(name != NULL);
    assert(ops != NULL);
    /*设备属性初始化*/
    elab_device_attr_t attr =
    {
        .name = (char *)name,
        .sole = true,
        .type = ELAB_DEVICE_PIN,
    };
    //设备层注册
    elab_device_register(&me->super, &attr);
    me->super.user_data = user_data;
    me->super.ops = NULL;

    //pin接口初始化
    me->ops = ops;
    //使用初始化函数
    me->ops->init(me);
    //初始时不对pin模式进行设置
    me->mode = PIN_MODE_MAX;
    elab_err_t ret = me->ops->get_status(me, &me->status);
    elab_assert(ret == ELAB_OK);
}

/**
  * @brief  eLab pin's status getting function.
  * @param  me      this pointer
  * @retval The pin's status.
  * @note pin模式设置
  */
void elab_pin_set_mode(elab_device_t * const me, uint8_t mode)
{
    assert(me != NULL);

    elab_pin_t *pin = (elab_pin_t *)me;
    if (pin->mode != mode)
    {
        pin->ops->set_mode(pin, mode); //调用底层接口
        pin->mode = mode; //设置模式
    }
}


/**
  * @brief  eLab pin's status getting function.
  * @param  me      this pointer
  * @retval The pin's status.
  * @note 读取pin状态
  */
bool elab_pin_get_status(elab_device_t *const me)
{
    assert(me != NULL);
    assert(ELAB_PIN_CAST(me)->mode < PIN_MODE_MAX);

    elab_pin_t *pin = (elab_pin_t *)me;

    if (pin->mode <= PIN_MODE_INPUT_PULLDOWN)  //上下拉模式才可以对io数据读取
    {
        bool status;
        elab_err_t ret = pin->ops->get_status(pin, &status); //调用底层接口
        if (ret == ELAB_OK)
        {
            pin->status = status;
        }
    }
    
    return pin->status;
}

/**
  * @brief  eLab pin's status turning on function.
  * @param  me      this pointer
  * @param  status  the input pin status.
  * @retval None.
  * @note 设置pin状态
  */
void elab_pin_set_status(elab_device_t *const me, bool status)
{
    assert(me != NULL);
    assert_name(ELAB_PIN_CAST(me)->mode == PIN_MODE_OUTPUT_PP ||
                ELAB_PIN_CAST(me)->mode == PIN_MODE_OUTPUT_OD,
                me->attr.name);

    elab_pin_t *pin = ELAB_PIN_CAST(me);
    if (status != pin->status)    //防止重复设置
    {
        elab_err_t ret = pin->ops->set_status(pin, status); //调用底层接口
        if (ret == ELAB_OK)
        {
            pin->status = status;
        }
    }
}

#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */
