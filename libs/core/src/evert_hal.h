#ifndef EVERT_HAL_H_
#define EVERT_HAL_H_

#include <arm_math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stm32g4xx_hal.h>

#include "_conf_evert_hal.h"
#include "evert_hal_adc.h"
#include "evert_hal_dwt.h"
#include "debugging.h"
#include "task_scheduler.h"

#if EVERT_HAL_CONF_EMC230X_ENABLE
#include "emc230x.h"
#endif

#if EVERT_HAL_CONF_FZ2812_ENABLE
#include "fz2812.h"
#endif

#endif // EVERT_HAL_H_