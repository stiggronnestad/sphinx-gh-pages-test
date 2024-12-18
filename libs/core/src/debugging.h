#ifndef DEBUGGING_H_
#define DEBUGGING_H_

#include <stdio.h>
#include <stdarg.h>
#include "_conf_evert_hal.h"

#if EVERT_HAL_CONF_DEBUGGING
extern volatile int break_points;
__noopt void EVERT_HAL_BreakPoint(const char *format, ...);
#else
#define EVERT_HAL_BreakPoint(format, ...) void
#endif

#endif