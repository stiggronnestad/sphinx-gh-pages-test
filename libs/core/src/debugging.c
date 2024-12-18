#include "debugging.h"

#ifdef EVERT_HAL_CONF_DEBUGGING
volatile int break_points = 0;
__noopt void EVERT_HAL_BreakPoint(const char *format, ...)
{
    va_list args;
    char buffer[256];
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    printf("%s", buffer);
    va_end(args);

    break_points++;
    __asm("BKPT #0");
}
#endif