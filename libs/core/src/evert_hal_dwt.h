#ifndef EVERT_HAL_DWT_H_
#define EVERT_HAL_DWT_H_

extern uint32_t DwtCycleCounterResult;

void EVERT_HAL_DWT_EnableCycleCounter(void);
void EVERT_HAL_DWT_Start(void);
uint32_t EVERT_HAL_DWT_Stop(void);

#endif // EVERT_HAL_DWT_H_