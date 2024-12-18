#ifndef EVERT_HAL_GPIO_DEFINITION_H_
#define EVERT_HAL_GPIO_DEFINITION_H_

#include <stm32g4xx_hal.h>

typedef struct
{
    GPIO_TypeDef *port;
    const uint16_t pin;
} EVERT_HAL_GpioDefinitionTypeDef;

#endif // EVERT_HAL_GPIO_DEFINITION_H_