/**
 ******************************************************************************
 * @file    boost_converter.c
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

// std/ST includes
#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_hrtim.h"

// Evert lib includes
#include "gpio_definition.h"
#include "status_register.h"

// Boost Converter includes
#include "boost_converter.h"

// Calibrations
EVERT_BOOST_CONVERTER_ConfigCurrentCalibrationTypeDef calibration_current;
EVERT_BOOST_CONVERTER_ConfigVoltageCalibrationTypeDef calibration_voltage;

// Calibrations
EVERT_BOOST_CONVERTER_ConfigCurrentConstraintsTypeDef constraints_current;
EVERT_BOOST_CONVERTER_ConfigTemperatureConstraintsTypeDef constraints_temperature;
EVERT_BOOST_CONVERTER_ConfigVoltageConstraintsTypeDef constraints_voltage;

EVERT_HAL_GpioDefinitionTypeDef EVERT_BOOST_CONVERTER_GPIO_DEF_ID_SELECTION = {GPIOB, GPIO_PIN_5};

// BOOST CONVERTER Objects
EVERT_BOOST_CONVERTER_TimeTypeDef time;
EVERT_BOOST_CONVERTER_IoStateTypeDef io_state = {GPIO_PIN_RESET};
volatile EVERT_BOOST_CONVERTER_MpptStateTypeDef mppt_state;

// CAN Handler
#define EVERT_CONSTRAINT_CAN_BUFFER_SIZE 16
EVERT_CAN_HandlerTypeDef can_handler;
EVERT_CAN_FifoBufferItemTypeDef rx_fifo_items[EVERT_CONSTRAINT_CAN_BUFFER_SIZE];
EVERT_CAN_FifoBufferItemTypeDef tx_fifo_items[EVERT_CONSTRAINT_CAN_BUFFER_SIZE];

// External Peripherals
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern FDCAN_HandleTypeDef hfdcan1;
extern HRTIM_HandleTypeDef hhrtim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

static volatile bool adc_completed[1] = {false};

/// @brief Main function for the boost converter
/// @details Initializes the boost converter and starts the main loop
/// @return 0 if successful
int EVERT_BOOST_CONVERTER_main()
{
    HAL_DMA_RegisterCallback(&hdma_adc1, HAL_DMA_XFER_ERROR_CB_ID, HAL_DMA_ErrorCallback);

    // Check the ID selection
    io_state.id_selection = HAL_GPIO_ReadPin(EVERT_BOOST_CONVERTER_GPIO_DEF_ID_SELECTION.port, EVERT_BOOST_CONVERTER_GPIO_DEF_ID_SELECTION.pin);

    // Initialize the time
    time.start_time = HAL_GetTick();

    // Initialize sub-systems
    EVERT_BOOST_CONVERTER_InitCalibrations();
    EVERT_BOOST_CONVERTER_InitConstraints();
    EVERT_BOOST_CONVERTER_InitFilters();
    EVERT_BOOST_CONVERTER_InitAlarmRegister();
    EVERT_DEVICE_SetVersionInfo(DEVICE_VERSION_MAJOR, DEVICE_VERSION_MINOR, DEVICE_VERSION_PATCH);
    EVERT_DEVICE_Init();

    // Start the ADCs
    EVERT_HAL_ADC_Start(&hadc1, (uint16_t *)adc1_buffer, EVERT_CONSTANT_BC_ADC1_CONVERSION_COUNT);

    // Start the timers
    HAL_TIM_Base_Start_IT(&htim2); // 10 kHz (HF ISR)
    HAL_TIM_Base_Start_IT(&htim3); // 100 Hz (LF ISR)

    // Start PWM output/timer
    HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1);
    HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_TIMER_A);

    // Start the mppt-control
    EVERT_BOOST_CONVERTER_SetDutyCycle(0.0f);
    EVERT_BOOST_CONVERTER_MpptInit();

    EVERT_CAN_DeviceIdentifierTypeDef device_id = io_state.id_selection == GPIO_PIN_RESET ? CAN_DEVICE_IDENTIFIER_BOOST_CONVERTER1 : CAN_DEVICE_IDENTIFIER_BOOST_CONVERTER2;
    EVERT_CAN_Handler_Init(&hfdcan1, &can_handler, device_id, rx_fifo_items, tx_fifo_items, EVERT_CONSTRAINT_CAN_BUFFER_SIZE);

    return 0;
}

/// @brief Main loop for the boost converter
/// @details Runs the main loop for the boost converter
void EVERT_BOOST_CONVERTER_loop()
{
    time.current_time = HAL_GetTick();
    time.elapsed_time = time.current_time - time.start_time;
    time.delta_time = time.current_time - time.last_time;

    EVERT_DEVICE_Update(time.elapsed_time, time.delta_time);

    // Process the CAN messages
    EVERT_CAN_ProcessBufferStatusTypeDef rxStatus = EVERT_CAN_Handler_ProcessRxBuffer(&can_handler);

    if (rxStatus != CAN_PBS_OK && rxStatus != CAN_PBS_IDLE && rxStatus != CAN_PBS_PROCESSING_RECEIVED_DATA)
    {
        EVERT_HAL_BreakPoint("Error processing RX buffer\n");
    }

    EVERT_CAN_ProcessBufferStatusTypeDef txStatus = EVERT_CAN_Handler_ProcessTxBuffer(&can_handler);

    if (txStatus != CAN_PBS_OK && txStatus != CAN_PBS_IDLE && txStatus != CAN_PBS_WAITING_FOR_INTERNAL_BUFFER)
    {
        EVERT_HAL_BreakPoint("Error processing TX buffer\n");
    }

    time.last_time = time.current_time;
}

void EVERT_BOOST_CONVERTER_SetDutyCycle(float32_t duty_cycle)
{
    // Check constraints for duty cycle
    if (duty_cycle > EVERT_CONSTRAINT_BC_DUTY_CYCLE_MAX)
    {
        duty_cycle = EVERT_CONSTRAINT_BC_DUTY_CYCLE_MAX;
    }
    else if (duty_cycle < EVERT_CONSTRAINT_BC_DUTY_CYCLE_MIN)
    {
        duty_cycle = EVERT_CONSTRAINT_BC_DUTY_CYCLE_MIN;
    }

    // Set the new duty cycle
    mppt_state.duty_cycle = duty_cycle;

    // Constant timer data
    const uint32_t HRTIMER_PERIOD = HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].PERxR;
    const uint32_t HRTIMER_COMPARE_MIN = 0;
    const uint32_t HRTIMER_COMPARE_MAX = HRTIMER_PERIOD;

    // New compare value based on duty cycle
    uint32_t compare_value = (uint32_t)(HRTIMER_PERIOD * duty_cycle);

    // Clamp the compare value
    if (compare_value < HRTIMER_COMPARE_MIN)
    {
        compare_value = HRTIMER_COMPARE_MIN;
    }
    else if (compare_value > HRTIMER_COMPARE_MAX)
    {
        compare_value = HRTIMER_COMPARE_MAX;
    }

    // Set the new compare value
    __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, compare_value);
    hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CNTxR = 0;
}

// __weak Callbacks - Alarms
void __overrides EVERT_BOOST_CONVERTER_AlarmChangedState(EVERT_BOOST_CONVERTER_AlarmIndexTypeDef alarm_index, bool is_set)
{
    UNUSED(alarm_index);
    UNUSED(is_set);

    // If there are no alarms set, the device is running
    if (EVERT_SR_GetBitOnCount(&alarm_register) == 0)
    {
        mppt_state.status = BCS_RUNNING;
    }
    // If there are any warning alarms set, the device should throttle down
    else if (EVERT_SR_IsAnyBitSet(&alarm_register, BOOST_CONVERTER_ALARMS_ThrottleDown, 7))
    {
        mppt_state.status = BCS_THROTTLE_DOWN;
    }
    // If there are any critical alarms set, the device should be in standby
    else if (EVERT_SR_IsAnyBitSet(&alarm_register, BOOST_CONVERTER_ALARMS_Standby, 7))
    {
        mppt_state.status = BCS_STANDBY;
    }
}

// __weak Callbacks - CAN
void __overrides EVERT_CAN_OnMessageReceived(EVERT_CAN_HandlerTypeDef *handler, const EVERT_CAN_FrameTypeDef frame)
{
    UNUSED(handler);
    UNUSED(frame);

    if (frame.data.length > 0)
    {
        uint32_t method = frame.data.data[0];
        uint32_t param1 = frame.data.data[1];

        if (method == 1)
        {
            mppt_state.status = param1 == 0 ? BCS_STANDBY : BCS_RUNNING;
        }
    }

    // if (frame.identifier.target_id == &handler->identifier)
    // {
    // }
    // EVERT_HAL_BreakPoint("CAN Message Received\n");
}

void __overrides EVERT_CAN_OnErrorReceived(EVERT_CAN_HandlerTypeDef *handler)
{
    UNUSED(handler);

    EVERT_HAL_BreakPoint("CAN Error Received\n");
}

// __weak Callbacks - State
void __overrides EVERT_DEVICE_Derived_OnDeviceStateChange(const EVERT_DEVICE_StateTypeDef new_state, const EVERT_DEVICE_StateTypeDef old_state)
{
    UNUSED(new_state);
    UNUSED(old_state);
}

void __overrides EVERT_DEVICE_Derived_OnEnterState_BootingComms()
{
    EVERT_DEVICE_State_Set(SS_INTERNAL, DS_HANDSHAKE_ACKNOWLEDGED);
}

// __weak Callbacks - Task Scheduler
void __overrides EVERT_TASK_SCHEDULER_OnTaskSendAnnouncement(void)
{
}
void __overrides EVERT_TASK_SCHEDULER_OnTaskSendData(void)
{
}
void __overrides EVERT_TASK_SCHEDULER_OnTaskSendDeviceStatus(void)
{
}
void __overrides EVERT_TASK_SCHEDULER_OnTaskSendPing(void)
{
    // EVERT_CAN_Identifier_SetMessageId(&handler->identifier, MI_PING);
    // EVERT_CAN_Identifier_SetPriority(&handler->identifier, MP_HIGH);
    // return EVERT_CAN_Handler_Transmit(handler, sizeof(EVERT_CAN_PingMessageTypeDef), (uint8_t *)message);

    // Send a ping message
    uint8_t data[7] = {0};
    EVERT_CAN_FifoStatusTypeDef status = EVERT_CAN_Handler_Transmit(&can_handler, sizeof(data), &data);
    if (status != CAN_FS_OK)
    {
        EVERT_HAL_BreakPoint("CAN Error: Ping\n");
    }
}

// Peripheral Callbacks
void EVERT_BOOST_CONVERTER_ISR_10KHZ_IRQHandler()
{
    if (!adc_completed[0])
    {
        return;
    }

    EVERT_BOOST_CONVERTER_Readings();

    // Set ADC conversion flag
    adc_completed[0] = false;
}

void EVERT_BOOST_CONVERTER_ISR_100HZ_IRQHandler()
{
    // Check the alarms/constraints
    EVERT_BOOST_CONVERTER_AlarmCheck();

    // Run the MPPT algorithm
    EVERT_BOOST_CONVERTER_MpptRun();
}

void __overrides HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == &hadc1)
    {
        adc_completed[0] = true;
    }
}

void __overrides EVERT_INVERTER_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    UNUSED(hfdcan);
    EVERT_CAN_Handler_Receive(&can_handler, RxFifo0ITs);
}

// Error Callbacks
void EVERT_BOOST_CONVERTER_hal_error(char *error_message)
{
    EVERT_BOOST_CONVERTER_CommonErrorCallback(error_message);
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    UNUSED(hadc);
    EVERT_BOOST_CONVERTER_CommonErrorCallback("ADC Error\n");
}

void HAL_DMA_ErrorCallback(DMA_HandleTypeDef *hdma)
{
    UNUSED(hdma);
    EVERT_BOOST_CONVERTER_CommonErrorCallback("DMA Error\n");
}

void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan)
{
    UNUSED(hfdcan);
    EVERT_BOOST_CONVERTER_CommonErrorCallback("FDCAN Error\n");
}

void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue)
{
    UNUSED(ReturnValue);
    EVERT_BOOST_CONVERTER_CommonErrorCallback("FLASH Error\n");
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    UNUSED(hi2c);
    EVERT_BOOST_CONVERTER_CommonErrorCallback("I2C Error\n");
}

void HAL_TIM_ErrorCallback(TIM_HandleTypeDef *htim)
{
    UNUSED(htim);
    EVERT_BOOST_CONVERTER_CommonErrorCallback("TIM Error\n");
}

void EVERT_BOOST_CONVERTER_CommonErrorCallback(char *error_message)
{
    EVERT_HAL_BreakPoint(error_message);

    __disable_irq();

    while (1)
    {
    }
}