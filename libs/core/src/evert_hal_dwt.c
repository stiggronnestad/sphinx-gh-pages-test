#include <stm32g4xx_hal.h>
#include "evert_hal_dwt.h"

uint32_t DwtCycleCounterResult = 0;
static uint32_t _dwt_start = 0;

void EVERT_HAL_DWT_EnableCycleCounter(void)
{
    // Enable the DWT cycle counter
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk; // Enable the TRCENA bit in the DEMCR register (Trace Control Register)
    DWT->CYCCNT = 0;                                // Reset the cycle counter
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            // Enable the cycle counter
}

void EVERT_HAL_DWT_Start(void)
{
    _dwt_start = DWT->CYCCNT;
}

uint32_t EVERT_HAL_DWT_Stop(void)
{
    DwtCycleCounterResult = DWT->CYCCNT - _dwt_start;
    return DWT->CYCCNT - _dwt_start;
}