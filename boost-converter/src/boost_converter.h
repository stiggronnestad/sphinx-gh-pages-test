/**
 ******************************************************************************
 * @file    boost_converter.h
 * @author  Evert Firmware Team
 * @brief   Functions and control loops for the boost converter.
 *          * Config: _conf_evert_boost_converter.h
 *          * Related:
 *              * boost_converter_alarms.h (alarms) | boost_converter_alarms.c
 *              * boost_converter_calibration.h
 *              * boost_converter_constraints.h
 *              * boost_converter_mppt.h (MPPT) | boost_converter_mppt.c
 *              * boost_converter_readings.h (readings) | boost_converter_readings.c
 *
 ******************************************************************************
 **/

#ifndef EVERT_BOOST_CONVERTER_MAIN_H_
#define EVERT_BOOST_CONVERTER_MAIN_H_

// std/ST includes
#include <stdbool.h>
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_hrtim.h"
#include "stm32g4xx_ll_cordic.h"
#include "adc.h"
#include "cordic.h"
#include "dma.h"
#include "fdcan.h"
#include "gpio.h"
#include "hrtim.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"

// Evert lib includes
#include "can_handler.h"
#include "evert_device.h"
#include "evert_hal.h"
#include "gpio_definition.h"
#include "task_scheduler.h"

// Boost Converter includes
#include "_conf_evert_device.h"
#include "_conf_evert_hal.h"
#include "_conf_evert_boost_converter.h"
#include "boost_converter_alarms.h"
#include "boost_converter_calibration.h"
#include "boost_converter_constraints.h"
#include "boost_converter_mppt.h"
#include "boost_converter_readings.h"

/// @brief Device major version for the boost converter
#define DEVICE_VERSION_MAJOR 2
/// @brief Device minor version for the boost converter
#define DEVICE_VERSION_MINOR 13
/// @brief Device path version for the boost converter
#define DEVICE_VERSION_PATCH 33

#define EVERT_BOOST_CONVERTER_EMA(new_value, prev_value, multiplier) (prev_value = multiplier * new_value + (1.0f - multiplier) * prev_value)
#define EVERT_BOOST_CONVERTER_CLAMP(value, min, max) (value = (value < min) ? min : ((value > max) ? max : value))

/// @brief Time structure for the boost converter
/// @details Contains the start time, current time, elapsed time, last time, and delta time
typedef struct
{
    uint32_t start_time;
    uint32_t current_time;
    uint32_t elapsed_time;
    uint32_t last_time;
    uint32_t delta_time;
} EVERT_BOOST_CONVERTER_TimeTypeDef;

/// @brief IO state structure for the boost converter
/// @details Contains the ID selection state
typedef struct
{
    GPIO_PinState id_selection;
} EVERT_BOOST_CONVERTER_IoStateTypeDef;

/// @brief Status enumeration for the boost converter
/// @details Contains the status of the boost converter
typedef enum
{
    BCS_STANDBY = 0,
    BCS_RUNNING = 1,
    BCS_THROTTLE_DOWN = 2,
} EVERT_BOOST_CONVERTER_StatusTypeDef;

/// @brief MPPT state structure for the boost converter
/// @details Contains the status, duty cycle, perturb step, observe interval, observe timer, current perturb step, previous duty cycle, and previous power
typedef struct
{
    EVERT_BOOST_CONVERTER_StatusTypeDef status;
    float32_t duty_cycle;
    float32_t perturb_step;
    uint32_t observe_interval;
    float32_t observe_timer;
    float32_t current_perturb_step;
    float32_t previous_duty_cycle;
    float32_t previous_power;
} EVERT_BOOST_CONVERTER_MpptStateTypeDef;

extern EVERT_BOOST_CONVERTER_TimeTypeDef time;
extern EVERT_BOOST_CONVERTER_IoStateTypeDef io_state;
extern volatile EVERT_BOOST_CONVERTER_MpptStateTypeDef mppt_state;

// Main
int EVERT_BOOST_CONVERTER_main();
void EVERT_BOOST_CONVERTER_loop();

void EVERT_BOOST_CONVERTER_SetDutyCycle(float32_t duty_cycle);

// __weak Callbacks - CAN
void __overrides EVERT_CAN_OnMessageReceived(EVERT_CAN_HandlerTypeDef *handler, const EVERT_CAN_FrameTypeDef frame);
void __overrides EVERT_CAN_OnErrorReceived(EVERT_CAN_HandlerTypeDef *handler);

// __weak Callbacks - State
void __overrides EVERT_DEVICE_Derived_OnDeviceStateChange(const EVERT_DEVICE_StateTypeDef new_state, const EVERT_DEVICE_StateTypeDef old_state);
void __overrides EVERT_DEVICE_Derived_OnEnterState_BootingComms();

// __weak Callbacks - Task Scheduler
void __overrides EVERT_TASK_SCHEDULER_OnTaskSendAnnouncement();
void __overrides EVERT_TASK_SCHEDULER_OnTaskSendData();
void __overrides EVERT_TASK_SCHEDULER_OnTaskSendDeviceStatus();
void __overrides EVERT_TASK_SCHEDULER_OnTaskSendPing();

// Peripheral Callbacks
void EVERT_BOOST_CONVERTER_ISR_10KHZ_IRQHandler();
void EVERT_BOOST_CONVERTER_ISR_100HZ_IRQHandler();
void __overrides HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);
void EVERT_INVERTER_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);

// Error Callbacks
void EVERT_BOOST_CONVERTER_hal_error(char *error_message);
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc);
void HAL_DMA_ErrorCallback(DMA_HandleTypeDef *hdma);
void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan);
void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue);
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c);
void HAL_TIM_ErrorCallback(TIM_HandleTypeDef *htim);
void EVERT_BOOST_CONVERTER_CommonErrorCallback(char *error_message);

#endif // EVERT_BOOST_CONVERTER_MAIN_H_