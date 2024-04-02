
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef SIMU_IO_DRIVER_PIN_H
#define SIMU_IO_DRIVER_PIN_H

/* include ------------------------------------------------------------------ */
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* public function ---------------------------------------------------------- */
void simu_io_new(const char *name, bool is_out);
void simu_in_set_status(const char *name, bool status);
bool simu_out_get_status(const char *name);

#ifdef __cplusplus
}
#endif

#endif  /* SIMU_IO_DRIVER_PIN_H */

/* ----------------------------- end of file -------------------------------- */
