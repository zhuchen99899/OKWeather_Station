/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab/common/elab_common.h"
#include "elab/elib/elib_queue.h"
#include "stm32g0xx_hal.h"

/* private config ----------------------------------------------------------- */
#define ELAB_DEBUG_UART_ID                      (1)

#define ELAB_DEBUG_UART_BUFFER_TX               (1024)//512
#define ELAB_DEBUG_UART_BUFFER_RX               (16)

/* private defines ---------------------------------------------------------- */
#if (ELAB_DEBUG_UART_ID == 1)

#define USARTx                                  USART1
#define __HAL_RCC_USARTx_CLK_ENABLE             __HAL_RCC_USART1_CLK_ENABLE
#define __HAL_RCC_GPIO_CLK_ENABLE               __HAL_RCC_GPIOA_CLK_ENABLE
#define USARTx_PIN                              (GPIO_PIN_9 | GPIO_PIN_10)
#define USARTx_PORT                             GPIOA
#define GPIO_AF_USART                           GPIO_AF1_USART1

#else


#endif

#if (ELAB_DEBUG_UART_ID != 1) 
#error "The debug uart ID must be set to 1 !"
#endif

/* private variables -------------------------------------------------------- */
UART_HandleTypeDef huart;

static elib_queue_t queue_rx;
static uint8_t buffer_rx[ELAB_DEBUG_UART_BUFFER_RX];
static elib_queue_t queue_tx;
static uint8_t buffer_tx[ELAB_DEBUG_UART_BUFFER_TX];
static uint8_t byte_recv;

/* public functions --------------------------------------------------------- */
/**
  * @brief  Initialize the elab debug uart.
  * @param  buffer  this pointer
  * @retval Free size.
  */
void elab_debug_uart_init(uint32_t baudrate)
{
    huart.Instance = USARTx;
    huart.Init.BaudRate = baudrate;
    huart.Init.WordLength = UART_WORDLENGTH_8B;
    huart.Init.StopBits = UART_STOPBITS_1;
    huart.Init.Parity = UART_PARITY_NONE;
    huart.Init.Mode = UART_MODE_TX_RX;
    huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart.Init.OverSampling = UART_OVERSAMPLING_16;
    huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart.Init.ClockPrescaler = UART_PRESCALER_DIV1;
    huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;

    HAL_UART_Init(&huart);
    HAL_UART_Receive_IT(&huart, &byte_recv, 1);
    
    elib_queue_init(&queue_rx, buffer_rx, ELAB_DEBUG_UART_BUFFER_RX);
    elib_queue_init(&queue_tx, buffer_tx, ELAB_DEBUG_UART_BUFFER_TX);
}

/**
  * @brief  Send data to the debug uart.
  * @param  buffer  this pointer
  * @retval Free size.
  */
int16_t elab_debug_uart_send(void *buffer, uint16_t size)
{
    int16_t ret = 0;
    uint8_t byte = 0;

    HAL_NVIC_DisableIRQ(USART1_IRQn);
    if (elib_queue_is_empty(&queue_tx))
    {
        ret = elib_queue_push(&queue_tx, buffer, size);
        if (elib_queue_pull(&queue_tx, &byte, 1) == 1)
        {
            HAL_UART_Transmit_IT(&huart, &byte, 1);
        }
    }
    else
    {
        ret = elib_queue_push(&queue_tx, buffer, size);
    }
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    return ret;
}

/**
  * @brief  Initialize the elab debug uart.
  * @param  buffer  this pointer
  * @retval Free size.
  */
int16_t elab_debug_uart_receive(void *buffer, uint16_t size)
{
    int16_t ret = 0;

    HAL_NVIC_DisableIRQ(USART1_IRQn);
    ret = elib_queue_pull_pop(&queue_rx, buffer, size);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    return ret;
}

/**
  * @brief  Clear buffer of the elab debug uart.
  * @param  buffer  this pointer
  * @retval Free size.
  */
void elab_debug_uart_buffer_clear(void)
{
    HAL_NVIC_DisableIRQ(USART1_IRQn);

    elib_queue_clear(&queue_rx);
    elib_queue_clear(&queue_tx);

    HAL_NVIC_EnableIRQ(USART1_IRQn);
}

/* private functions -------------------------------------------------------- */
/**
  * @brief  The weak UART tx callback function in HAL library.
  * @param  uartHandle  UART handle.
  * @retval None.
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    uint8_t byte = 0;
    
    if (UartHandle->Instance == USARTx)
    {
        elib_queue_pop(&queue_tx, 1);
        if (elib_queue_pull(&queue_tx, &byte, 1))
        {
            HAL_UART_Transmit_IT(&huart, &byte, 1);
        }
    }
}

/**
  * @brief  The weak UART rx callback function in HAL library.
  * @param  uartHandle  UART handle.
  * @retval None.
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
    if (UartHandle->Instance == USARTx)
    {
        HAL_UART_Receive_IT(&huart, &byte_recv, 1);
        elib_queue_push(&queue_rx, &byte_recv, 1);
    }
}

/**
  * @brief  The weak UART initialization function in HAL library.
  * @param  uartHandle  UART handle.
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (uartHandle->Instance == USARTx)
    {
        /* USART clock enable */
        __HAL_RCC_USARTx_CLK_ENABLE();
        __HAL_RCC_GPIO_CLK_ENABLE();

        /** USART GPIO Configuration
            PA9     ------> USART1_RX
            PA10     ------> USART1_TX
            

        */
        GPIO_InitStruct.Pin = USARTx_PIN;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF_USART;
        HAL_GPIO_Init(USARTx_PORT, &GPIO_InitStruct);

        /* USART3 interrupt Init */
        HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
}

/**
  * @brief This function handles USART3 and USART4 interrupts.
  */
void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart);
}

/* ----------------------------- end of file -------------------------------- */
