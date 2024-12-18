extern "C"
{
#include "evert_main.h"
}

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <vector>

#include "boost_converter.h"
#include "status_led.h"
#include "fir_filter.h"
#include "can_handler.h"
#include "device_can_identifier.h"
#include "device_version_info.h"
#include "device_hal_transfer_object.h"
#include "device_hal_utilities.h"
#include "device_state.h"
#include "ping_message.h"
#include "data_conversion.h"

using namespace evert;
using namespace evert::core;
using namespace evert::core::devices;
using namespace evert::protocols;

DEVICE_HAL_TRANSFER_OBJECT *hto;

scheduling::Scheduler scheduler;
comms::CanHandler can_handler;
comms::SerialHandler serial_handler;
can::DeviceVersionInfo device_version_info = can::DeviceVersionInfo(DEVICE_VERSION_MAJOR, DEVICE_VERSION_MINOR, DEVICE_VERSION_PATCH);
evert::BoostConverter boost_converter(device_version_info, scheduler, can_handler, serial_handler);

volatile bool adc_completed[1] = {false};
uint32_t last_tick = 0;

int EVERT_DEVICE_main(DEVICE_HAL_TRANSFER_OBJECT *hal_transfer_object)
{
    hto = hal_transfer_object;
    RegisterDmaErrorCallbacks();

    // Init the boost converter with the HAL transfer object
    boost_converter.InitHto(hal_transfer_object);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);

    return 0;
}

void EVERT_DEVICE_loop()
{
    for (;;)
    {
        uint32_t tick = HAL_GetTick();

        float delta_time_ms = (tick - last_tick);
        scheduler.Update(delta_time_ms);
        last_tick = tick;

        boost_converter.Loop(tick);
    }
}

void EVERT_DEVICE_TIM2_IRQHandler()
{
    // ISR Frequency = 25 kHz
    // Prescaler = 0, Counter = 6800-1
    // ISR_Time = 1 / 25 kHz = 40 us
    // CPU Cycles per ISR = 40 us * 170 MHz = 6800 cycles

    // Toggle PA10
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_10);
    boost_converter.ISR1_HighFrequency(25000, 6800);
}

void EVERT_DEVICE_TIM3_IRQHandler()
{
    // ISR Frequency = 100 Hz
    // Prescaler = 17000, Counter = 100-1
    // ISR_Time = 1 / 100 Hz = 10 ms
    // CPU Cycles per ISR = 10 ms * 170 MHz = 1,700,000 cycles

    // Toggle PA10
    // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_10);

    if (adc_completed[0])
    {
        // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_10);
        boost_converter.ISR2_LowFrequency(100, 1700000);
        adc_completed[0] = false;
    }
}

void EVERT_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    UNUSED(hfdcan);
    HAL_StatusTypeDef status = can_handler.Receive(RxFifo0ITs);

    if (status == HAL_OK)
    {
        boost_converter.ClearDeviceAlarm(can::DeviceAlarmRegister1Index::kHalCanError);
    }
    else
    {
        boost_converter.RegisterDeviceAlarm(can::DeviceAlarmRegister1Index::kHalCanError);
    }
}

void EVERT_HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == hto->huart3->Instance)
    {
        serial_handler.TxCpltCallback();
    }
}

void EVERT_HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == hto->huart3->Instance)
    {
        serial_handler.RxCpltCallback();
    }
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    // Check for ADC overrun
    if (__HAL_ADC_GET_FLAG(hadc, ADC_FLAG_OVR))
    {
        boost_converter.RegisterDeviceAlarm(can::DeviceAlarmRegister1Index::kHalAdcOverrun);
        __HAL_ADC_CLEAR_FLAG(hadc, ADC_FLAG_OVR);
    }
    // else
    // {
    // TODO: Clear the alarm? Some sort of logging must be implemented.
    //     inverter.GetAlarmRegister1().ClearAlarm(can::DeviceAlarmRegister1Index::kAlarmHalAdcOverrun);
    // }

    if (hadc == hto->hadc1)
    {
        adc_completed[0] = true;
    }
}

void RegisterDmaErrorCallbacks()
{
    if (devices::HTO_DMA_USED(hto->hdma_adc1))
    {
        HAL_DMA_RegisterCallback(hto->hdma_adc1, HAL_DMA_XFER_ERROR_CB_ID, HAL_DMA_ErrorCallback);
    }
    if (devices::HTO_DMA_USED(hto->hdma_adc2))
    {
        HAL_DMA_RegisterCallback(hto->hdma_adc2, HAL_DMA_XFER_ERROR_CB_ID, HAL_DMA_ErrorCallback);
    }
    if (devices::HTO_DMA_USED(hto->hdma_adc3))
    {
        HAL_DMA_RegisterCallback(hto->hdma_adc3, HAL_DMA_XFER_ERROR_CB_ID, HAL_DMA_ErrorCallback);
    }
}

//
// __Weak Implementation of the HAL Error Callbacks
//
void EVERT_DEVICE_hal_error()
{
    // Probably pointless
    boost_converter.RegisterDeviceAlarm(can::DeviceAlarmRegister1Index::kHalError);

    __disable_irq();

    while (1)
    {
    }
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    UNUSED(hadc);
    boost_converter.RegisterDeviceAlarm(can::DeviceAlarmRegister1Index::kHalAdcError);
}

void HAL_DMA_ErrorCallback(DMA_HandleTypeDef *hdma)
{
    UNUSED(hdma);
    boost_converter.RegisterDeviceAlarm(can::DeviceAlarmRegister1Index::kHalDmaError);
}

void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan)
{
    UNUSED(hfdcan);
    boost_converter.RegisterDeviceAlarm(can::DeviceAlarmRegister1Index::kHalCanError);
}

void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue)
{
    UNUSED(ReturnValue);
    boost_converter.RegisterDeviceAlarm(can::DeviceAlarmRegister1Index::kHalFlashError);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    UNUSED(hi2c);
    boost_converter.RegisterDeviceAlarm(can::DeviceAlarmRegister1Index::kHalI2cError);
}

void HAL_TIM_ErrorCallback(TIM_HandleTypeDef *htim)
{
    UNUSED(htim);
    boost_converter.RegisterDeviceAlarm(can::DeviceAlarmRegister1Index::kHalTimError);
}

void EVERT_HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    // UART can crash from time to time, if sending more data than the buffer can handle
    // for example.

    boost_converter.RegisterDeviceAlarm(can::DeviceAlarmRegister1Index::kHalUartError);

    // FE - framing error seems to be common when faults occur
    if (huart->ErrorCode == HAL_UART_ERROR_FE)
    {
        __HAL_UART_CLEAR_FEFLAG(huart);
    }

    // ORE - overrun error
    if (huart->ErrorCode == HAL_UART_ERROR_ORE)
    {
        __HAL_UART_CLEAR_OREFLAG(huart);
    }

    // Try to reset the UART
    if (serial_handler.Reset() == HAL_OK)
    {
        boost_converter.ClearDeviceAlarm(can::DeviceAlarmRegister1Index::kHalUartError);
    }
}