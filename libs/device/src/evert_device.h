#ifndef EVERT_DEVICE_H_
#define EVERT_DEVICE_H_

/// @brief STM32 HAL Library Includes
#include <stdbool.h>
#include <stm32g4xx_hal.h>

/// @brief EVERT Library Includes
#include "_conf_evert_device.h"
#include "evert_device_alarm_register1_index.h"
#include "evert_device_state.h"
#include "evert_device_version_info.h"
#include "status_register.h"
#include "task_scheduler.h"

/// @brief Device State Scope Definition
typedef enum
{
    SS_INTERNAL = 0,
    SS_PROPAGATED = 1,
    SS_RESULT = 2
} EVERT_DEVICE_StateScopeTypeDef;

/// @brief Device State Group Definition
typedef struct
{
    EVERT_DEVICE_StateTypeDef Internal;
    EVERT_DEVICE_StateTypeDef Propagated;
    EVERT_DEVICE_StateTypeDef Result;
} EVERT_DEVICE_StateGroupTypeDef;

/// @brief Device Peripheral Status Definition
typedef struct
{
    // ADC
    uint32_t Adc1ErrorCode; // ADC Error Code
    uint32_t Adc2ErrorCode; // ADC Error Code
    uint32_t Adc3ErrorCode; // ADC Error Code
    uint32_t Adc4ErrorCode; // ADC Error Code
    uint32_t Adc5ErrorCode; // ADC Error Code

    // Clock
    uint32_t ClockFrequency; // System Clock Frequency
    uint8_t ClockHseStatus;  // HSE Clock Status (1 = On)
    uint8_t ClockHseReady;   // HSE Clock Ready Status (1 = Ready)

    // CAN
    uint8_t Can1Rec;        // Receive Error Counter
    uint8_t Can1Tec;        // Transmit Error Counter
    uint32_t Can1Psr;       // Protocol Status Register
    uint32_t Can1ErrorCode; // CAN Error Code
    uint8_t Can2Rec;        // Receive Error Counter
    uint8_t Can2Tec;        // Transmit Error Counter
    uint32_t Can2Psr;       // Protocol Status Register
    uint32_t Can2ErrorCode; // CAN Error Code
    uint8_t Can3Rec;        // Receive Error Counter
    uint8_t Can3Tec;        // Transmit Error Counter
    uint32_t Can3Psr;       // Protocol Status Register
    uint32_t Can3ErrorCode; // CAN Error Code

    // DMA
    uint32_t DmaAdc1ErrorCode; // DMA Error Code
    uint8_t DmaAdc1State;
    uint32_t DmaAdc2ErrorCode; // DMA Error Code
    uint8_t DmaAdc2State;
    uint32_t DmaAdc3ErrorCode; // DMA Error Code
    uint8_t DmaAdc3State;

    // Flash
    uint8_t FlashErrorCode; // Flash memory error code

    // I2C
    uint32_t I2c1ErrorCode; // I2C Error Code

    // UART
    uint32_t Lpuart1ErrorCode; // UART Error Code
    uint32_t Huart3ErrorCode;  // UART3 Error Code

    // Watchdog
    uint8_t WatchdogStatus; // Watchdog Timer Status (1 = Triggered)
    uint32_t ResetCause;    // Reset Cause

} EVERT_DEVICE_PeripheralStatusTypeDef;

/// @brief Base Device Type Definition
typedef struct
{
    EVERT_DEVICE_VersionInfoTypeDef DeviceVersionInfo;
    EVERT_SR_RegisterTypeDefinitionTypeDef AlarmRegister1; // Alarm Register 1
    EVERT_DEVICE_PeripheralStatusTypeDef PeripheralStatus;
    EVERT_DEVICE_StateGroupTypeDef StateGroup;
} EVERT_DEVICE_BaseDeviceTypeDef;

void EVERT_DEVICE_Derived_OnDeviceStateChange(const EVERT_DEVICE_StateTypeDef new_state, const EVERT_DEVICE_StateTypeDef old_state);
void EVERT_DEVICE_Derived_OnEnterState_Booting();
void EVERT_DEVICE_Derived_OnEnterState_BootingAdc();
void EVERT_DEVICE_Derived_OnEnterState_BootingComms();
void EVERT_DEVICE_Derived_OnEnterState_BootingDone();
void EVERT_DEVICE_Derived_OnEnterState_HandshakeAnnouncing();
void EVERT_DEVICE_Derived_OnEnterState_HandshakeAcknowledged(const EVERT_DEVICE_StateTypeDef previous_state);
void EVERT_DEVICE_Derived_OnEnterState_Operational(const EVERT_DEVICE_StateTypeDef previous_state);
void EVERT_DEVICE_Derived_OnEnterState_OperationalWarning(const EVERT_DEVICE_StateTypeDef previous_state);
void EVERT_DEVICE_Derived_OnEnterState_NonOperational(const EVERT_DEVICE_StateTypeDef previous_state);
void EVERT_DEVICE_Derived_OnEnterState_EmergencyShutdown(const EVERT_DEVICE_StateTypeDef previous_state);

void EVERT_DEVICE_Init();
void EVERT_DEVICE_SetVersionInfo(const uint8_t major, const uint8_t minor, const uint8_t patch);
void EVERT_DEVICE_Update(const uint32_t elapsed_ms, const uint32_t delta_ms);

EVERT_DEVICE_StateTypeDef EVERT_DEVICE_Alarm_Check();
void EVERT_DEVICE_State_Set(const EVERT_DEVICE_StateScopeTypeDef scope, const EVERT_DEVICE_StateTypeDef state);
EVERT_DEVICE_StateTypeDef EVERT_DEVICE_State_Get(const EVERT_DEVICE_StateScopeTypeDef scope);

// inline bool EVERT_DEVICE_Peripheral_ADC_USED(ADC_HandleTypeDef *adc) { return adc != NULL; }
// inline bool EVERT_DEVICE_Peripheral_CAN_USED(FDCAN_HandleTypeDef *can) { return can != NULL; }
// inline bool EVERT_DEVICE_Peripheral_DMA_USED(DMA_HandleTypeDef *dma) { return dma != NULL; }
// inline bool EVERT_DEVICE_Peripheral_HRTIM_USED(HRTIM_HandleTypeDef *tim) { return tim != NULL; }
// inline bool EVERT_DEVICE_Peripheral_TIM_USED(TIM_HandleTypeDef *tim) { return tim != NULL; }
// inline bool EVERT_DEVICE_Peripheral_I2C_USED(I2C_HandleTypeDef *i2c) { return i2c != NULL; }
// inline bool EVERT_DEVICE_Peripheral_UART_USED(UART_HandleTypeDef *uart) { return uart != NULL; }

void EVERT_DEVICE_PeripheralStatus_PopulateObject(EVERT_DEVICE_PeripheralStatusTypeDef *status);
bool EVERT_DEVICE_PeripheralStatus_HasError(EVERT_DEVICE_PeripheralStatusTypeDef *status);

#endif // EVERT_DEVICE_H_