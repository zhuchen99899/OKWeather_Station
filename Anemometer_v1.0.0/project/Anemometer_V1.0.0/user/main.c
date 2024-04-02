/**
 * @file main.c
 * @author zc (387646983@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-01-10
 * 
 * @copyright Copyright (c) 2024
 * 
 */
/* Includes ------------------------------------------------------------------*/
#include "bsp/bsp.h"
#include "elab/common/elab_export.h"

/* public functions --------------------------------------------------------- */
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    bsp_init();

    elab_run();
}

/* ----------------------------- end of file -------------------------------- */





