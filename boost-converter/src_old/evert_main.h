#ifndef EVERT_DEVICE_MAIN_H_
#define EVERT_DEVICE_MAIN_H_

#define DEVICE_VERSION_MAJOR 2
#define DEVICE_VERSION_MINOR 13
#define DEVICE_VERSION_PATCH 33

#include "main.h"
#include "device_hal_transfer_object.h"

// Main
int EVERT_DEVICE_main(DEVICE_HAL_TRANSFER_OBJECT *hal_transfer_object);
void EVERT_DEVICE_loop();

// Peripheral Callbacks
void EVERT_DEVICE_TIM2_IRQHandler();
void EVERT_DEVICE_TIM3_IRQHandler();
void EVERT_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);
void EVERT_HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);
void EVERT_HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc);
void RegisterDmaErrorCallbacks();

// Error Callbacks
void EVERT_DEVICE_hal_error();
void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc);
void HAL_DMA_ErrorCallback(DMA_HandleTypeDef *hdma);
void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan);
void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue);
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c);
void HAL_TIM_ErrorCallback(TIM_HandleTypeDef *htim);
void EVERT_HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);

#endif // EVERT_DEVICE_MAIN_H_