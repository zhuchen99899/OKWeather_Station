/**
 * @file system_test.c
 * @author zc (387646983@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-01-10
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "shell.h"
#include "stm32g0xx.h"
#include "../../common/elab_assert.h"

#ifdef __cplusplus
extern "C" {
#endif
ELAB_TAG("system_test");

/**
 * @brief  test shell to Get the systemclock cfg object
 * 
 */
void get_systemclock_cfg(void)
{
	RCC_OscInitTypeDef RCCcfg;
	RCC_ClkInitTypeDef Rcc_clock_cfg;
	uint32_t flash_state;
	
	HAL_RCC_GetOscConfig(&RCCcfg);
		
	
	elog_debug("HSEState=%d,HSICalibrationValue=%d,HSIDiv=%d,HSIState=%d,LSEState=%d,LSIState=%d,OscillatorType=0x%x\r\n",
	RCCcfg.HSEState,
	RCCcfg.HSICalibrationValue,
	RCCcfg.HSIDiv,
	RCCcfg.HSIState,
	RCCcfg.LSEState,
	RCCcfg.LSIState,
	RCCcfg.OscillatorType);
	
	elog_debug("PLLState=%d,PLLSource=%d,PLLM=%d,PLLN=%d,PLLP=0x%x,PLLR=0x%x\r\n",
	RCCcfg.PLL.PLLState,
	RCCcfg.PLL.PLLSource,
	RCCcfg.PLL.PLLM,
	RCCcfg.PLL.PLLN,
	RCCcfg.PLL.PLLP,
	RCCcfg.PLL.PLLR);
	
	
	HAL_RCC_GetClockConfig(&Rcc_clock_cfg,&flash_state);
	elog_debug("AHBCLKDivider=0x%x,APB1CLKDivider=0x%x,ClockType=0x%x,SYSCLKSource=0x%x",
	Rcc_clock_cfg.AHBCLKDivider,
	Rcc_clock_cfg.APB1CLKDivider,
	Rcc_clock_cfg.ClockType,
	Rcc_clock_cfg.SYSCLKSource);

}

//导出到命令列表里
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), get_systemclock_cfg, get_systemclock_cfg, get_systemclock_cfg);

/**
 * @brief Get the sysfrq object
 * 
 */
void get_sysfrq(void)
{
	uint32_t SYSCLKFreq,HCLKFreq,PCLK1Freq;
	SYSCLKFreq=HAL_RCC_GetSysClockFreq();
	HCLKFreq=HAL_RCC_GetHCLKFreq();
	PCLK1Freq=HAL_RCC_GetPCLK1Freq();

  elog_debug("SYSCLKFreq= %d,HCLKFreq= %d,PCLK1Freq= %d\r\n",SYSCLKFreq,HCLKFreq,PCLK1Freq);
}


//导出到命令列表里
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0)|SHELL_CMD_TYPE(SHELL_TYPE_CMD_FUNC), get_sysfrq, get_sysfrq, get_sysfrq);


#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */



