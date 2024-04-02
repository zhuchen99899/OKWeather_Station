/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#ifndef ELAB_H
#define ELAB_H

/* includes ----------------------------------------------------------------- */
#include "common/elab_def.h"
#include "common/elab_assert.h"
#include "common/elab_export.h"
#include "common/elab_common.h"

#if (ELAB_QPC_EN != 0)
#include "3rd/qpc/include/qpc.h"
#include "event_def.h"
#endif

#if (ELAB_RTOS_CMSIS_OS_EN != 0)
#include "os/cmsis_os.h"
#endif

#if (ELAB_RTOS_BASIC_OS_EN != 0)
#include "basic_os.h"
#endif

#if (ELAB_RTOS_BASIC_OS_EN != 0)
#include "basic_os.h"
#endif

#endif

/* ----------------------------- end of file -------------------------------- */
