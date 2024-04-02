/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab_device.h"
#include "elab_device_def.h"
#include "../common/elab_assert.h"
#include "../common/elab_log.h"
#include "../os/cmsis_os.h"

ELAB_TAG("EdfDevice");

/* private function prototypes ---------------------------------------------- */
static void _add_device(elab_device_t *me); //将设备加入到设备表中
static osMutexId_t _edf_mutex(void); //创建edf操作锁，在对device操作过程进行锁处理

/* private variables -------------------------------------------------------- */
static uint32_t _edf_device_count = 0; // 设备计数
static elab_device_t *_edf_table[ELAB_DEV_NUM_MAX]; //设备表
static osMutexId_t _mutex_edf = NULL; //edf锁，在对device进行操作过程进行锁处理

/**
 * The edf global mutex attribute.
 */
//锁属性 CMSIS_RTOS API 需要的参数
/*attr_bits 设置为 osMutexRecursive。
同一任务使用多个互斥锁且不会自锁
拥有这个互斥锁的任务lock数会递增
互斥锁需要释放多次直至lock数为0
互斥锁lock数为0时，其他任务可以获得这个互斥锁
attr_bits 设置为 osMutexPrioInherit。
如果设置优先级继承策略，那么优先级翻转问题会得到解决
如果没有优先级继承策略，那么可能高优先级任务会被低优先级任务给阻塞
*/
static const osMutexAttr_t _mutex_attr_edf =
{
    .name = "mutex_edf", //锁名称
    .attr_bits = osMutexPrioInherit | osMutexRecursive,
    .cb_mem = NULL,
    .cb_size = 0, 
};

/* public function ---------------------------------------------------------- */
/**
 * This function registers a device driver with specified name.
 * @param dev the pointer of device driver structure
 * @param name the device driver's name
 * @return None.
 * @note 设备层设备注册，由中间抽象设备层(HAL)调用
 */
void elab_device_register(elab_device_t *me, elab_device_attr_t *attr)
{
    //形参检查
    assert(me != NULL);
    assert(attr != NULL);
    assert(attr->name != NULL);
    assert_name(elab_device_find(attr->name) == NULL, attr->name);

    /* Edf mutex locking. */
    osStatus_t ret = osOK;
    osMutexId_t mutex = _edf_mutex(); //创建edf_mutex 
    ret = osMutexAcquire(mutex, osWaitForever);//获取edf_mutex锁,锁住整个device注册的过程
    assert(ret == osOK);

    /* Set the device data. */
    memcpy(&me->attr, attr, sizeof(elab_device_attr_t)); //注册设备属性
    me->enable_count = 0;      
    me->lock_count = 0;                       //初始化 使能计数以及锁计数
    me->mutex = osMutexNew(&_mutex_attr_edf); //创建device 设备锁
    assert(me->mutex != NULL);

    /* Add the device the edf table. */
    memcpy(&me->attr, attr, sizeof(elab_device_attr_t));
    _add_device(me);                           //将设备锁加入列表

    /* Edf mutex unlocking. */
    ret = osMutexRelease(mutex);         //解锁def_mutex
    assert(ret == osOK);
}

/**
 * @brief This function unregisters a device with the device handle.
 * @param me   the pointer of device driver structure
 * @retval None.
 * @note 设备层解除设备注册，linux等系统会调用，在RTOS中一般设备注册后不再解除注册
 */
void elab_device_unregister(elab_device_t *me)
{
    elab_assert(me != NULL);
    elab_assert(!elab_device_is_enabled(me));

    /* Edf mutex locking. */
    osStatus_t ret = osOK;
    osMutexId_t mutex = _edf_mutex();
    ret = osMutexAcquire(mutex, osWaitForever);
    assert(ret == osOK);

    for (uint32_t i = 0; i < ELAB_DEV_NUM_MAX; i ++)
    {
        if (_edf_table[i] == me)
        {
            osStatus_t ret = osMutexDelete(me->mutex);
            elab_assert(ret == osOK);
            me->mutex = NULL;
            _edf_table[i] = NULL;
            _edf_device_count --;
            break;
        }
    }

    /* Edf mutex unlocking. */
    ret = osMutexRelease(mutex);
    assert(ret == osOK);
}

/**
 * @brief Get the count number in device framework management.
 * @retval Count number of devices.
 * @note 获取已经注册设备个数
 */
uint32_t elab_device_get_number(void)
{
    uint32_t num = 0;

    /* Edf mutex locking. */
    osStatus_t ret = osOK;
    osMutexId_t mutex = _edf_mutex();
    ret = osMutexAcquire(mutex, osWaitForever);
    assert(ret == osOK);

    num = _edf_device_count;

    /* Edf mutex unlocking. */
    ret = osMutexRelease(mutex);
    assert(ret == osOK);

    return num;
}

/**
 * This function check the given name is the device's name or not.
 * @param me    Device handle.
 * @param name  Device name.
 * @return True or false.
 * @note 获取设备名称
 */
bool elab_device_of_name(elab_device_t *me, const char *name)
{
    bool of_the_name = false;

    elab_device_lock(me);
    if (strcmp(me->attr.name, name) == 0)
    {
        of_the_name = true;
    }
    elab_device_unlock(me);

    return of_the_name;
}

/**
 * This function finds a device driver by specified name.
 * @param name  Device name.
 * @return Device handle. If not found, return NULL.
 * @note 获取已经注册的设备句柄 在应用层使用设备层函数时，通过找到句柄后再通过接口操作设备
 */
elab_device_t *elab_device_find(const char *name)
{
    if (_edf_table[0] != NULL)
    assert_name(_edf_table[0]->attr.name != NULL, name);
    assert(name != NULL);
    
    /* Edf mutex locking. */
    osStatus_t ret = osOK;
    osMutexId_t mutex = _edf_mutex();
    ret = osMutexAcquire(mutex, osWaitForever);
    assert(ret == osOK);

    /* Find the device */
    elab_device_t *me = NULL;
    for (uint32_t i = 0; i < ELAB_DEV_NUM_MAX; i++)
    {
        /* No device yet in the device table. */
        if (_edf_table[i] == NULL)
        {
            break;
        }
        elab_assert(_edf_table[i]->attr.name != NULL);
        /* Device matching */
        if (strcmp(_edf_table[i]->attr.name, name) == 0)
        {
            me = _edf_table[i];
            break;
        }
    }

    /* Edf mutex unlocking. */
    ret = osMutexRelease(mutex);
    assert(ret == osOK);

    return me;
}

/**
 * This function check one device name is valid or not.
 * @param name  Device name.
 * @return Valid if true and invalid if false.
 * @note 判断已注册设备是否合法 在参数检查中使用
 */
bool elab_device_valid(const char *name)
{
    return elab_device_find(name) == NULL ? false : true;
}

/**
 * @brief This function check one device name is sole or not.
 * @param name  Device name.
 * @return Valid if true and invalid if false.
 * @note 判断设备是否独占
 */
bool elab_device_is_sole(elab_device_t *me)
{
    elab_device_lock(me);
    bool enable_status = me->attr.sole;
    elab_device_unlock(me);

    return enable_status;
}

/**
 * @brief Check the device is in test mode or not.
 * @param dev       the pointer of device driver structure
 * @retval True or false.
 * @note 判断设备是否处于测试模式
 */
bool elab_device_is_test_mode(elab_device_t *dev)
{
    return (dev->thread_test != NULL) ? true : false;
}

/**
 * @brief Set the test mode for the device.
 * @param dev       The pointer of device driver structure
 * @retval None.
 * @note 设置设备为测试模式
 */
void elab_device_set_test_mode(elab_device_t *dev)
{
    elab_assert(dev != NULL);

    elab_device_lock(dev);
    dev->thread_test = osThreadGetId();
    elab_device_unlock(dev);
}

/**
 * @brief Set the normal mode for the device.
 * @param dev       the pointer of device driver structure
 * @retval None.
 * @note 设置设备为正常模式
 */
void elab_device_set_normal_mode(elab_device_t *dev)
{
    elab_assert(dev != NULL);

    elab_device_lock(dev);
    dev->thread_test = NULL;
    elab_device_unlock(dev);
}

/**
 * @brief This function check one device is enabled or not.
 * @param name  Device name.
 * @return Valid if true and invalid if false.
 * @note 判断设备是否开启
 */
bool elab_device_is_enabled(elab_device_t *me)
{
    assert(me != NULL);

    elab_device_lock(me);
    bool enable_status = me->enable_count > 0 ? true : false;
    elab_device_unlock(me);

    return enable_status;
}

/**
 * Lock or unlock one device to ensure its thread-safety.
 * @param me        Device handle.
 * @param status    The locking status.
 * @return None.
 * @note 锁定设备 or 解锁设备
 */
void __device_mutex_lock(elab_device_t *me, bool status)
{
    elab_assert(me != NULL);
    elab_assert(me->mutex != NULL);

    osStatus_t ret = osOK;

    if (status)
    {
        ret = osMutexAcquire(me->mutex, osWaitForever);
    }
    else
    {
        ret = osMutexRelease(me->mutex);
    }
    assert(ret == osOK);
}

/**
 * This function will enable or disable a device
 * @param me        Device handle.
 * @param status    The locking status.
 * @return the result
 * @note 设备使能
 */
elab_err_t __device_enable(elab_device_t *me, bool status)
{
    assert(me != NULL);
    assert(me->ops != NULL);
    assert(me->ops->enable != NULL);

    elab_device_lock(me);
    
    if (me->attr.sole)
    {
        if (status)
        {
            assert_name(me->enable_count == 0, me->attr.name);
        }
        else
        {
            assert_name(me->enable_count > 0, me->attr.name);
        }
    }
    else
    {
        assert_name(me->enable_count < UINT8_MAX, me->attr.name);
    }
    
    elab_err_t ret = ELAB_OK;
    if (status && me->enable_count == 0)
    {
        ret = me->ops->enable(me, true);
    }
    else if (!status && me->enable_count == 1)
    {
        ret = me->ops->enable(me, false);
    }
    me->enable_count = status ? (me->enable_count + 1) : (me->enable_count - 1);

    elab_device_unlock(me);

    return ret;
}

/**
 * This function will read some data from a device.
 *
 * @param dev       the pointer of device driver structure
 * @param buffer    the data buffer to save read data
 * @param size      the size of buffer
 *
 * @return the actually read size on successful, otherwise negative returned.
 * @note 设备读接口
 *
 */
int32_t elab_device_read(elab_device_t *me,
                            uint32_t pos, void *buffer, uint32_t size)
{
    assert(me != NULL);
    assert(me->enable_count != 0);
    assert(me->ops != NULL);
    assert(me->ops->read != NULL);

    int32_t ret = 0;
    if (elab_device_is_test_mode(me))
    {
        ret = ELAB_OK;
        goto exit;
    }

    ret = me->ops->read(me, pos, buffer, size);

exit:
    return ret;
}

/**
 * This function will write some data to a device.
 *
 * @param me        the pointer of device driver structure.
 * @param buffer    the data buffer to be written to device
 * @param size      the size of buffer
 *
 * @return The actually written size on successful, otherwise negative returned.
 * @note 设备写接口
 */
int32_t elab_device_write(elab_device_t *me,
                            uint32_t pos, const void *buffer, uint32_t size)
{
    assert(me != NULL);
    assert(me->enable_count != 0);
    assert(me->ops != NULL);
    assert(me->ops->write != NULL);

    int32_t ret = 0;
    if (elab_device_is_test_mode(me))
    {
        ret = ELAB_OK;
        goto exit;
    }

    ret = me->ops->write(me, pos, buffer, size);

exit:
    return ret;
}

/* private functions -------------------------------------------------------- */
static osMutexId_t _edf_mutex(void)
{
    if (_mutex_edf == NULL)
    {
        _mutex_edf = osMutexNew(&_mutex_attr_edf);
        assert(_mutex_edf != NULL);
    }

    return _mutex_edf;
}

static void _add_device(elab_device_t *me)
{
    assert(_edf_device_count < ELAB_DEV_NUM_MAX);

    if (_edf_device_count == 0)
    {
        for (uint32_t i = 0; i < ELAB_DEV_NUM_MAX; i ++)
        {
            _edf_table[i] = NULL;
        }
    }
    _edf_table[_edf_device_count ++] = me;
}

/* ----------------------------- end of file -------------------------------- */
