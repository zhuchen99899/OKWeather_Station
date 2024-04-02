/**
 * @file drv_spi_bus.c
 * @author zc (387646983@qq.com)
 * @brief 
 * @version 0.1
 * @date 2024-01-10
 * 
 * @copyright Copyright (c) 2024
 * 
 */
/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"
#include "elab/common/elab_export.h"
#include "elab/common/elab_common.h"
#include "elab/common/elab_assert.h"
#include "elab/edf/normal/elab_spi.h"
#include "elab/edf/normal/elab_pin.h"
#include "./export/export.h"

#ifdef __cplusplus
extern "C" {
#endif

ELAB_TAG("DriverSpiBus");

/* private function prototypes ---------------------------------------------- */
static elab_err_t _config(elab_spi_t *const me, elab_spi_config_t *config);
static elab_err_t _xfer(elab_spi_t * const me, elab_spi_msg_t *message);

/* private variables -------------------------------------------------------- */
static elab_spi_bus_t spi_bus;
static SPI_HandleTypeDef hspi1;
static DMA_HandleTypeDef hdma_spi1_tx;
static DMA_HandleTypeDef hdma_spi1_rx;

static elab_spi_bus_ops_t spi_bus_ops =
{
    .config = _config,
    .xfer = _xfer,
};


/* public function ---------------------------------------------------------- */
/* SPI1 init function */
/**
 * @brief SPI总线底层驱动初始化
 * 
 */
static void spi_bus_export(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_SPI1_CLK_ENABLE();



    /* DMA1_Channel2_3_IRQn interrupt configuration */

    HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);


    /**SPI1 GPIO Configuration
    PA1     ------> SPI1_SCK
    PA6     ------> SPI1_MISO
    PA2     ------> SPI1_MOSI
    */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* SPI1 DMA Init */
    /* SPI1_TX Init */
    hdma_spi1_tx.Instance = DMA1_Channel3;
    hdma_spi1_tx.Init.Request = DMA_REQUEST_SPI1_TX;
    hdma_spi1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi1_tx.Init.Mode = DMA_NORMAL;
    hdma_spi1_tx.Init.Priority = DMA_PRIORITY_LOW;
    HAL_DMA_Init(&hdma_spi1_tx);

    __HAL_LINKDMA(&hspi1, hdmatx,hdma_spi1_tx);

    /* SPI1_RX Init */
    hdma_spi1_rx.Instance = DMA1_Channel2;
    hdma_spi1_rx.Init.Request = DMA_REQUEST_SPI1_RX;
    hdma_spi1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_spi1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi1_rx.Init.Mode = DMA_NORMAL;
    hdma_spi1_rx.Init.Priority = DMA_PRIORITY_HIGH;
    HAL_DMA_Init(&hdma_spi1_rx);

    __HAL_LINKDMA(&hspi1, hdmarx, hdma_spi1_rx);


    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 7;
    hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    HAL_SPI_Init(&hspi1);



    elab_spi_bus_register(&spi_bus, "SPI1", &spi_bus_ops, &hspi1);
}


INIT_EXPORT(spi_bus_export, EXPORT_LEVEL_SPI_BUS);


/**
 * @brief _config 底层接口
 * 
 * @param me  spi 设备抽象 
 * @param config spi 配设修改
 * @return elab_err_t 
 */
static elab_err_t _config(elab_spi_t *const me, elab_spi_config_t *config)
{
    elab_assert(me != NULL);
    
    (void)me;
    (void)config;

    /* TODO */
    return ELAB_OK;
}

/**
 * @brief _xfer 发送接收 接口
 * 
 * @param me spi设备抽象
 * @param message spi消息
 * @return elab_err_t 
 */
static elab_err_t _xfer(elab_spi_t * const me, elab_spi_msg_t *message)
{
    SPI_HandleTypeDef *spi = (SPI_HandleTypeDef *)me->bus->super.user_data;
    HAL_StatusTypeDef status = HAL_BUSY;
    
    if (message->buff_send != NULL && message->buff_recv != NULL) //边发边接收
    {
        status = HAL_SPI_TransmitReceive_DMA(spi,
                                                (uint8_t *)message->buff_send,
                                                (uint8_t *)message->buff_recv,
                                                message->size);
    }
    else if (message->buff_send == NULL) //接收
    {
        status = HAL_SPI_Receive_DMA(spi,
                                        (uint8_t *)message->buff_recv,
                                        message->size);
    }
    else if (message->buff_recv == NULL) //发送
    {
        status = HAL_SPI_Transmit_DMA(spi,
                                        (uint8_t *)message->buff_send,
                                        message->size);
    }
    else
    {
        elab_assert(false);
    }


    (void)status;
    
    return ELAB_OK;
}

/* private functions -------------------------------------------------------- */
/**
  * @brief This function handles DMA1 channel 2 and channel 3 interrupts.
  * DMA中断
  */

void DMA1_Channel2_3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&hdma_spi1_rx); //HAL接收中断
    HAL_DMA_IRQHandler(&hdma_spi1_tx);// HAL发送中断
}

/**
 * @brief halSPI发送完成中断
 * 
 * @param hspi spi句柄
 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    (void)hspi;
    elab_spi_bus_xfer_end(&spi_bus); //回调函数，由抽象层定义
}

/**
 * @brief halSPI接收完成中断
 * 
 * @param hspi 
 * 
 */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    (void)hspi;
    elab_spi_bus_xfer_end(&spi_bus);//回调函数，由抽象层定义
}

/**
 * @brief halSPI发送同时接收完成中断
 * 
 * @param hspi 
 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    (void)hspi;
    elab_spi_bus_xfer_end(&spi_bus);//回调函数，由抽象层定义
}


#ifdef __cplusplus
}
#endif

/* ----------------------------- end of file -------------------------------- */


