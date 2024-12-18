#include <arm_math.h>
#include <stm32g4xx_hal.h>
#include "evert_hal_adc.h"

HAL_StatusTypeDef EVERT_HAL_ADC_Start(ADC_HandleTypeDef *hadc, uint16_t *buffer, uint32_t conversion_count)
{
    if (HAL_ADCEx_Calibration_Start(hadc, ADC_SINGLE_ENDED) != HAL_OK)
    {
        return HAL_ERROR;
    }

    if (HAL_ADC_Start_DMA(hadc, (uint32_t *)buffer, conversion_count) != HAL_OK)
    {
        return HAL_ERROR;
    }

    return HAL_OK;
}

float32_t EVERT_HAL_ADC_Lerp(float adc, float slope, float y_intercept)
{
    return slope * adc + y_intercept;
}