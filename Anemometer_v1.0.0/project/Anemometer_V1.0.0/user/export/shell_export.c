/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* include ------------------------------------------------------------------ */
#include <stdio.h>
#include "elab/common/elab_common.h"
#include "elab/common/elab_export.h"
#include "elab/3rd/Shell/shell.h"
#include "elab/common/elab_log.h"

ELAB_TAG("ShellExport");

/* private config ----------------------------------------------------------- */
#define SHELL_POLL_PERIOD_MS                (10)
#define SHELL_BUFFER_SIZE                   (1024)//512

/* private variables -------------------------------------------------------- */
static Shell shell_uart;
static char shell_uart_buffer[SHELL_BUFFER_SIZE];

/* private functions -------------------------------------------------------- */
/**
  * @brief  Shell uart initialization.
  * @retval None
  */
static void shell_uart_init(void)
{
    elab_debug_uart_init(115200);

    shell_uart.read = (int16_t (*)(char *, uint16_t))elab_debug_uart_receive;
    shell_uart.write = (int16_t (*)(char *, uint16_t))elab_debug_uart_send;
    shellInit(&shell_uart, shell_uart_buffer, SHELL_BUFFER_SIZE);
    
    elog_debug("Shell uart init end.\r\n");
}
INIT_EXPORT(shell_uart_init, 0);

/* private functions -------------------------------------------------------- */
/**
  * @brief  Shell polling function.
  * @retval None
  */
static void shell_poll(void)
{
    char byte;
    while (shell_uart.read && shell_uart.read(&byte, 1) == 1)
    {
        shellHandler(&shell_uart, byte);
    }
}
POLL_EXPORT(shell_poll, SHELL_POLL_PERIOD_MS);

/* ----------------------------- end of file -------------------------------- */
