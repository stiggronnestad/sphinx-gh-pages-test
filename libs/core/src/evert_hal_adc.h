#ifndef EVERT_HAL_ADC_H_
#define EVERT_HAL_ADC_H_

#include <arm_math.h>
#include <stm32g4xx_hal.h>

HAL_StatusTypeDef EVERT_HAL_ADC_Start(ADC_HandleTypeDef *hadc, uint16_t *buffer, uint32_t conversion_count);

float32_t EVERT_HAL_ADC_Lerp(float adc, float slope, float y_intercept);

#endif // EVERT_HAL_ADC_H_