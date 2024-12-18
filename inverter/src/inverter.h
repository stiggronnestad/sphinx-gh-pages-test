#ifndef EVERT_INVERTER_MAIN_H_
#define EVERT_INVERTER_MAIN_H_

#include "adc.h"
#include "cordic.h"
#include "dma.h"
#include "fdcan.h"
#include "gpio.h"
#include "hrtim.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"

#include "_conf_evert_device.h"
#include "_conf_evert_hal.h"
#include "_conf_evert_inverter.h"
#include "evert_device.h"
#include "evert_hal.h"
#include "gpio_definition.h"
#include "inverter_alarm_index.h"
#include "inverter_calibration.h"
#include "inverter_constraints.h"
#include "inverter_grid.h"
#include "inverter_math.h"
#include "inverter_readings.h"
#include "inverter_transforms.h"
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_hrtim.h"
#include "stm32g4xx_ll_cordic.h"
#include "task_scheduler.h"

#define DEVICE_VERSION_MAJOR 2
#define DEVICE_VERSION_MINOR 13
#define DEVICE_VERSION_PATCH 33

typedef struct
{
    uint32_t start_time;
    uint32_t current_time;
    uint32_t elapsed_time;
    uint32_t last_time;
    uint32_t delta_time;
} EVERT_INVERTER_TimeTypeDef;

typedef struct
{
    GPIO_PinState pwm_ready_top1;
    GPIO_PinState pwm_ready_top2;
    GPIO_PinState pwm_ready_top3;
    GPIO_PinState pwm_ready_bot1;
    GPIO_PinState pwm_ready_bot2;
    GPIO_PinState pwm_ready_bot3;
    GPIO_PinState pwm_fault;
    GPIO_PinState pwm_fan_fault;
    GPIO_PinState pwm_fdcan_fault;
} EVERT_INVERTER_IoStateTypeDef;

extern EVERT_INVERTER_TimeTypeDef time;
extern EVERT_INVERTER_IoStateTypeDef io_state;
extern EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_ENABLE;
extern EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_RESET;
extern EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_READY_TOP1;
extern EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_READY_TOP2;
extern EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_READY_TOP3;
extern EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_READY_BOT1;
extern EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_READY_BOT2;
extern EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_READY_BOT3;
extern EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_FAULT;
extern EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_FAN_FAULT;
extern EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_FDCAN_FAULT;

// Main
int EVERT_INVERTER_main();
void EVERT_INVERTER_loop();

void GetDutyCycles(float32_t *duty_u, float32_t *duty_v, float32_t *duty_w);
float32_t EVERT_INVERTER_GetDutyCycleCcrByPercentage(const float32_t percentage);

void EVERT_INVERTER_SetDeadtime(const uint32_t timer_index, uint16_t deadtime);
void EVERT_INVERTER_SetDutyCycle(const float32_t duty_cycle_u, const float32_t duty_cycle_v, const float32_t duty_cycle_w);
void EVERT_INVERTER_SetPwmEnabled(const bool enabled);
void EVERT_INVERTER_SetStatusLed(const uint8_t index, const uint32_t color, const uint32_t time_on, const uint32_t time_off);

// __weak Callbacks - State
void __overrides EVERT_DEVICE_Derived_OnDeviceStateChange(const EVERT_DEVICE_StateTypeDef new_state, const EVERT_DEVICE_StateTypeDef old_state);
void __overrides EVERT_DEVICE_Derived_OnEnterState_BootingComms();

// __weak Callbacks - Task Scheduler
void __overrides EVERT_TASK_SCHEDULER_OnTaskSendAnnouncement();
void __overrides EVERT_TASK_SCHEDULER_OnTaskSendData();
void __overrides EVERT_TASK_SCHEDULER_OnTaskSendDeviceStatus();
void __overrides EVERT_TASK_SCHEDULER_OnTaskSendPing();

// Peripheral Callbacks
void EVERT_INVERTER_ISR_25KHZ_IRQHandler();
void EVERT_INVERTER_ISR_100HZ_IRQHandler();
void __overrides HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);

// Error Callbacks
void EVERT_INVERTER_hal_error(char *error_message);
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc);
void HAL_DMA_ErrorCallback(DMA_HandleTypeDef *hdma);
void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan);
void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue);
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c);
void HAL_TIM_ErrorCallback(TIM_HandleTypeDef *htim);
void EVERT_INVERTER_CommonErrorCallback(char *error_message);

#endif // EVERT_INVERTER_MAIN_H_