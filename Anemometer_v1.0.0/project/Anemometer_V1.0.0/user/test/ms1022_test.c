/**
 * @file ms1022_test.c
 * @author zc (387646983@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-01-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "shell.h"
#include "../../common/elab_assert.h"
#include "../../edf/elab_device.h"
#include "./device/elab_ms1022.h"
#include "./device/elab_anemometer.h"

/* private variables -------------------------------------------------------- */
ELAB_TAG("ms1022_test");
elab_device_t *ms1022 = NULL;

elab_device_t *anemometer =NULL;

elab_err_t test_ms1022_com(void)
{
ms1022=elab_device_find("ms1022");
elab_err_t elab_ret=ms1022_test_communication(ms1022);

if(elab_ret==ELAB_OK)
{
elog_debug("ms1022_test_communication is ok");

}
else
{

elog_error("ms1022_test_communication is error");
}

return elab_ret;

};
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), test_ms1022_com, test_ms1022_com, test_ms1022_com);


void read_all_ms1022cfg_regs(void)
{
ms1022=elab_device_find("ms1022");
elab_ms1022_read_all_cfgregs(ms1022);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), read_all_ms1022cfg_regs, read_all_ms1022cfg_regs, read_all_ms1022cfg_regs);

void read_ms1022_freq_corr_fact(void)
{
ms1022=elab_device_find("ms1022");
float data=elab_ms1022_freq_corr_fact(ms1022);
elog_debug("test=%f",data);   
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), read_ms1022_freq_corr_fact, read_ms1022_freq_corr_fact, read_ms1022_freq_corr_fact);

void new_test_func(uint8_t mode)
{

anemometer=elab_device_find("anemometer1");
switch (mode)
{
case 0:
   elab_anemometer_period(anemometer,500);
    break;
case 1:
    elab_anemometer_stop(anemometer);
    break;
default:
    break;
}

}

SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), new_test_func, new_test_func, new_test_func);

