/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */


#ifndef __RS485_H
#define __RS485_H

/* Includes ------------------------------------------------------------------*/
#include "../elab_device.h"
#include "../../os/cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Exported types ------------------------------------------------------------*/
typedef struct
{
    elab_device_t *serial;
    elab_device_t *pin_tx_en;
    bool tx_en_high_active;
    osMutexId_t mutex;

    void *user_data;
} rs485_t;

/* Exported functions --------------------------------------------------------*/
elab_err_t rs485_init(rs485_t *me,
                        const char *serial_name,
                        const char *pin_tx_en_name,
                        bool tx_en_high_active,
                        void *user_data);
elab_device_t *rs485_get_serial(rs485_t *me);

int32_t rs485_read(rs485_t *me, void *pbuf, uint32_t size);
int32_t rs485_write(rs485_t *me, const void *pbuf, uint32_t size);
int32_t rs485_write_time(rs485_t *me, const void *pbuf, uint32_t size, uint32_t time);

#ifdef __cplusplus
}
#endif

#endif /* __RS485_H */

/* ----------------------------- end of file -------------------------------- */
