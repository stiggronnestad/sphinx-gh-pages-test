/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LPUART1_RX_Pin GPIO_PIN_0
#define LPUART1_RX_GPIO_Port GPIOC
#define LPUART1_TX_Pin GPIO_PIN_1
#define LPUART1_TX_GPIO_Port GPIOC
#define CURRENT_V_Pin GPIO_PIN_2
#define CURRENT_V_GPIO_Port GPIOC
#define V_BUS_Pin GPIO_PIN_0
#define V_BUS_GPIO_Port GPIOA
#define V_BUS_MID_Pin GPIO_PIN_1
#define V_BUS_MID_GPIO_Port GPIOA
#define CURRENT_U_Pin GPIO_PIN_2
#define CURRENT_U_GPIO_Port GPIOA
#define LS1_SHUTDOWN_Pin GPIO_PIN_3
#define LS1_SHUTDOWN_GPIO_Port GPIOA
#define TEMPERATURE_U_Pin GPIO_PIN_4
#define TEMPERATURE_U_GPIO_Port GPIOA
#define LS2_SHUTDOWN_Pin GPIO_PIN_5
#define LS2_SHUTDOWN_GPIO_Port GPIOA
#define VOLTAGE_INVERTER_W_Pin GPIO_PIN_6
#define VOLTAGE_INVERTER_W_GPIO_Port GPIOA
#define VOLTAGE_GRID_U_Pin GPIO_PIN_7
#define VOLTAGE_GRID_U_GPIO_Port GPIOA
#define VOLTAGE_GRID_V_Pin GPIO_PIN_5
#define VOLTAGE_GRID_V_GPIO_Port GPIOC
#define VOLTAGE_INVERTER_V_Pin GPIO_PIN_0
#define VOLTAGE_INVERTER_V_GPIO_Port GPIOB
#define CURRENT_W_Pin GPIO_PIN_1
#define CURRENT_W_GPIO_Port GPIOB
#define VOLTAGE_GRID_W_Pin GPIO_PIN_2
#define VOLTAGE_GRID_W_GPIO_Port GPIOB
#define Filter_Temperature_U_Pin GPIO_PIN_8
#define Filter_Temperature_U_GPIO_Port GPIOE
#define TEMPERATURE_V_Pin GPIO_PIN_9
#define TEMPERATURE_V_GPIO_Port GPIOE
#define RESERVE_2_Pin GPIO_PIN_10
#define RESERVE_2_GPIO_Port GPIOE
#define RESERVE_3_Pin GPIO_PIN_11
#define RESERVE_3_GPIO_Port GPIOE
#define RESERVE_4_Pin GPIO_PIN_12
#define RESERVE_4_GPIO_Port GPIOE
#define TEMPERATURE_W_Pin GPIO_PIN_13
#define TEMPERATURE_W_GPIO_Port GPIOE
#define VOLTAGE_INVERTER_U_Pin GPIO_PIN_11
#define VOLTAGE_INVERTER_U_GPIO_Port GPIOB
#define Filter_Temperature_V_Pin GPIO_PIN_10
#define Filter_Temperature_V_GPIO_Port GPIOD
#define Filter_Temperature_W_Pin GPIO_PIN_11
#define Filter_Temperature_W_GPIO_Port GPIOD
#define AMBIENT_TEMPERATURE_Pin GPIO_PIN_13
#define AMBIENT_TEMPERATURE_GPIO_Port GPIOD
#define PWM_FAULT_Pin GPIO_PIN_12
#define PWM_FAULT_GPIO_Port GPIOA
#define FAN_CONTROL_1_Pin GPIO_PIN_10
#define FAN_CONTROL_1_GPIO_Port GPIOC
#define FAN_CONTROL_2_Pin GPIO_PIN_11
#define FAN_CONTROL_2_GPIO_Port GPIOC
#define FAN_CONTROL_3_Pin GPIO_PIN_12
#define FAN_CONTROL_3_GPIO_Port GPIOC
#define PWM_ENABLE_Pin GPIO_PIN_1
#define PWM_ENABLE_GPIO_Port GPIOD
#define GATE_DRIVER_RESET_Pin GPIO_PIN_2
#define GATE_DRIVER_RESET_GPIO_Port GPIOD
#define RGB_LEDS_Pin GPIO_PIN_3
#define RGB_LEDS_GPIO_Port GPIOD
#define PWM_READY_TOP1_Pin GPIO_PIN_4
#define PWM_READY_TOP1_GPIO_Port GPIOD
#define PWM_READY_TOP2_Pin GPIO_PIN_5
#define PWM_READY_TOP2_GPIO_Port GPIOD
#define PWM_READY_TOP3_Pin GPIO_PIN_6
#define PWM_READY_TOP3_GPIO_Port GPIOD
#define PWM_READY_BOT1_Pin GPIO_PIN_7
#define PWM_READY_BOT1_GPIO_Port GPIOD
#define PWM_READY_BOT2_Pin GPIO_PIN_3
#define PWM_READY_BOT2_GPIO_Port GPIOB
#define PWM_READY_BOT3_Pin GPIO_PIN_4
#define PWM_READY_BOT3_GPIO_Port GPIOB
#define FAN_FAULT_Pin GPIO_PIN_0
#define FAN_FAULT_GPIO_Port GPIOE
#define FDCAN_FAULT_Pin GPIO_PIN_1
#define FDCAN_FAULT_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
