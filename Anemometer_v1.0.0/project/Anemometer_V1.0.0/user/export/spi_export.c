
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* includes ----------------------------------------------------------------- */
#include "elab/common/elab_export.h"
#include "elab/edf/normal/elab_spi.h"
#include "export.h"
#include "./device/elab_ms1022.h"
#include "./device/elab_anemometer.h"
/* private variables -------------------------------------------------------- */
static elab_spi_t spi_ms1022;
static elab_spidev_ms1022_t ms1022;
static elab_anemometer_t anemometer;
/* public functions --------------------------------------------------------- */
static void driver_spi_export(void)
{
    elab_spi_config_t config =
    {
        .mode = ELAB_SPI_MODE_0,
        .data_width = 8,
        .max_hz = 1000000,
    };
    //注册 spi设备    //设备抽象    spi设备名称   spi总线     片选引脚       配设
    elab_spi_register(&spi_ms1022, "spi_ms1022", "SPI1", "pin_ms1022_cs", config);
}
INIT_EXPORT(driver_spi_export, EXPORT_LEVEL_SPI);


static void spi_device_export(void)
{
elab_ms1022_register(&ms1022,"ms1022","spi_ms1022","pin_ms1022_intn");
}

INIT_EXPORT(spi_device_export,EXPORT_LEVEL_USR_MS1022);

static void anemometer_export(void)
{

elab_anemometer_register(&anemometer,"anemometer1","ms1022","fire_74hc4052d","ut_receive_74hc4052d");    
}
INIT_EXPORT(anemometer_export,EXPORT_LEVEL_USR_ANEMOMETER);
/* ----------------------------- end of file -------------------------------- */
