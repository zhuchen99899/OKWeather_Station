#ifndef EXPORT_H
#define EXPORT_H

enum
{
    EXPORT_LEVEL_SHELL=0,      //低于此初始化等级时，在shell中不显示初始化过程

    EXPORT_LEVEL_ADC            = 1,
    EXPORT_LEVEL_PIN_MCU        = 1,
    EXPORT_LEVEL_PWM_MCU        = 1,
    EXPORT_LEVEL_SPI_BUS        = 1,
    EXPORT_LEVEL_I2C            = 2,
    EXPORT_LEVEL_SPI            = 2,
    EXPORT_LEVEL_LED            = 3,
    EXPORT_LEVEL_OLED           = 3,
    EXPORT_LEVEL_PIN_I2C        = 3,
    EXPORT_LEVEL_BUTTON         = 4,
    EXPORT_LEVEL_USR_74HC405RD  = 4,
    EXPORT_LEVEL_USR_MS1022     = 5,
    EXPORT_LEVEL_USR_ULTRASONIC = 6,
    EXPORT_LEVEL_USR_ANEMOMETER = 7

};

#endif
