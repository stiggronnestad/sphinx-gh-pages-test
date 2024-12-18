#include <functional>

#include "boost_converter_alarm_index.h"
#include "boost_converter_alarm_state_matrix.h"
#include "boost_converter_dio_register_index.h"
#include "boost_converter.h"
#include "boost_converter_config_message.h"
#include "boost_converter_measurements_message.h"
#include "boost_converter_mppt_message.h"
#include "boost_converter_settings.h"
#include "boost_converter_status_message.h"
#include "can_handler.h"
#include "can_frame_data.h"
#include "can_frame_data_conversion.h"
#include "device.h"
#include "device_devkit.h"
#include "lerp.h"
#include "mppt_status.h"
#include "ping_message.h"
#include "scheduler.h"
#include "status_led.h"
#include "status_register.h"
#include "task.h"
#include "uart_semantics.h"
#include "uart_frame_data.h"
#include "uart_frame_data_conversion.h"

using namespace evert::core;
using namespace evert::core::devices;
using namespace evert::protocols;

namespace evert
{
    BoostConverter::BoostConverter(const can::DeviceVersionInfo device_version_info, scheduling::Scheduler &scheduler, comms::CanHandler &can_handler, comms::SerialHandler &serial_handler)
        : devices::DevKit(device_version_info, scheduler, can_handler),
          serial_handler_(serial_handler),
          oscillation_detector_(10, 0.3)
    {
        // std::vector<float> fir_coeffs = {
        //     0.000375f, 0.001058f, 0.002922f, 0.006137f, 0.010850f,
        //     0.017175f, 0.025164f, 0.034800f, 0.045990f, 0.058561f,
        //     0.072277f, 0.086837f, 0.101886f, 0.117031f, 0.131853f,
        //     0.145927f, 0.158839f, 0.170199f, 0.179647f, 0.186859f,
        //     0.191565f, 0.193561f, 0.192717f, 0.188985f, 0.182393f,
        //     0.173052f, 0.161156f, 0.146968f, 0.130810f, 0.113048f,
        //     0.094072f, 0.074269f};

        filter_u_in_ = filters::MavgFilter(200);
        filter_u_out_ = filters::MavgFilter(200);
        filter_i_in_ = filters::MavgFilter(200);
        filter_coil_temp_ = filters::MavgFilter(200);
        filter_schottky_temp_ = filters::MavgFilter(200);
        filter_mosfet_temp_ = filters::MavgFilter(200);
    }

    void BoostConverter::InitHto(DEVICE_HAL_TRANSFER_OBJECT *hto)
    {
        HAL_ADCEx_Calibration_Start(hto->hadc1, ADC_SINGLE_ENDED);
        HAL_ADC_Start_DMA(hto->hadc1, (uint32_t *)adc1_buffer_, EVERT_SETTING_BC_ADC1_CONVERSION_COUNT);

        HAL_TIM_Base_Start_IT(hto->htim2);
        HAL_TIM_Base_Start_IT(hto->htim3);

        // Call base class InitHto
        devices::DevKit::InitHto(hto);
    }

    can::DeviceCanIdentifier BoostConverter::GetCanIdentifier()
    {
        if (can_identifier_ != can::DeviceCanIdentifier::kUnspecified)
        {
            return can_identifier_;
        }

        GPIO_PinState state = HAL_GPIO_ReadPin(GPIO_DEF_ID_SELECTION.port, GPIO_DEF_ID_SELECTION.pin);

        if (state == GPIO_PIN_SET)
        {
            // Pulled high = Boost Converter 2
            can_identifier_ = can::DeviceCanIdentifier::kBoostConverter2;
            bc_dio_register_.SetBit(can::BoostConverterDioRegisterIndex::kIdSelection);
        }
        else
        {
            // Pulled low = Boost Converter 1
            can_identifier_ = can::DeviceCanIdentifier::kBoostConverter1;
            bc_dio_register_.ClearBit(can::BoostConverterDioRegisterIndex::kIdSelection);
        }

        return can_identifier_;
    }

    void BoostConverter::Loop(uint32_t tick)
    {
        devices::DevKit::Loop(tick);
    }

    void BoostConverter::OnEnterState_BootingComms()
    {
        can::DeviceCanIdentifier can_identifier = GetCanIdentifier();

        // Init the CAN handler
        HAL_StatusTypeDef can_status = can_handler_.Init(hto_->hfdcan1, can_identifier);

        if (can_status != HAL_OK)
        {
            RegisterDeviceAlarm(can::DeviceAlarmRegister1Index::kHalUartError);
        }

        // Init the serial handler
        HAL_StatusTypeDef serial_status = serial_handler_.Init(hto_->huart3);

        if (serial_status != HAL_OK)
        {
            RegisterDeviceAlarm(can::DeviceAlarmRegister1Index::kHalUartError);
        }

        // Bind the OnRawUartDataReceived method to the SerialHandler
        auto on_raw_uart_data_received = std::bind(&BoostConverter::OnRawUartDataReceived, this, std::placeholders::_1, std::placeholders::_2);
        serial_handler_.SubscribeRaw(on_raw_uart_data_received);

        SetupMessages();
        SetupTasks();

        // Continue booting or go to done - up to each individual device
        // On booting done the device will enter the Announcing state so we skip it in the CCU
        SetDeviceState(DeviceStateScope::kInternal, can::DeviceState::kBootingDone);
    }

    void BoostConverter::OnEnterState_BootingDone()
    {
        devices::DevKit::OnEnterState_BootingDone();

        SetFrequency(50000);
        SetDutyCycle(0.0f);
        SetObserveInterval(500);
        SetPerturbStep(0.005f);
        current_perturb_step_ = 0.005f;
    }

    void BoostConverter::OnEnterState_HandshakeAcknowledged(const can::DeviceState previous_state)
    {
        devices::DevKit::OnEnterState_HandshakeAcknowledged(previous_state);

        task_send_data_->Resume();

        // TODO: Dev remove this and exchange for real handshake
        can_handler_.Send(current_constraints_);
        SendSerialMessageData(can::MessagePriority::kLow, can::MessageId::kBoostConverterConfigCurrentConstraints, serializing::DataConversion::ToBytes(current_constraints_));

        can_handler_.Send(voltage_constraints_);
        SendSerialMessageData(can::MessagePriority::kLow, can::MessageId::kBoostConverterConfigVoltageConstraints, serializing::DataConversion::ToBytes(voltage_constraints_));

        can_handler_.Send(temperature_constraints_);
        SendSerialMessageData(can::MessagePriority::kLow, can::MessageId::kBoostConverterConfigTemperatureConstraints, serializing::DataConversion::ToBytes(temperature_constraints_));

        can_handler_.Send(voltage_calibration_);
        SendSerialMessageData(can::MessagePriority::kLow, can::MessageId::kBoostConverterConfigVoltageCalibration, serializing::DataConversion::ToBytes(voltage_calibration_));

        can_handler_.Send(current_calibration_);
        SendSerialMessageData(can::MessagePriority::kLow, can::MessageId::kBoostConverterConfigCurrentCalibration, serializing::DataConversion::ToBytes(current_calibration_));
    }

    void BoostConverter::OnEnterState_Operational(const can::DeviceState previous_state)
    {
        UNUSED(previous_state);

        if (device_mode_ == can::DeviceMode::kAutomatic)
        {
            SetMpptStatus(can::MpptStatus::kSeeking);
            SetDutyCycle(0.1f);
            task_send_data_->ChangeInterval(EVERT_SETTING_BC_TASK_SEND_DATA_INTERVAL);
        }
    }

    void BoostConverter::OnEnterState_OperationalWarning(const can::DeviceState previous_state)
    {
        if (device_mode_ == can::DeviceMode::kAutomatic)
        {
            UNUSED(previous_state);
            SetMpptStatus(can::MpptStatus::kThrottleDown);
        }

        task_send_data_->ChangeInterval(EVERT_SETTING_BC_TASK_SEND_DATA_INTERVAL);
    }

    void BoostConverter::OnEnterState_NonOperational(const can::DeviceState previous_state)
    {
        if (device_mode_ == can::DeviceMode::kAutomatic)
        {
            UNUSED(previous_state);
            SetMpptStatus(can::MpptStatus::kStandby);
            SetDutyCycle(0.0f);
        }

        task_send_data_->ChangeInterval(EVERT_SETTING_BC_TASK_SEND_DATA_INTERVAL);
    }

    void BoostConverter::OnEnterState_EmergencyShutdown(const can::DeviceState previous_state)
    {
        if (device_mode_ == can::DeviceMode::kAutomatic)
        {
            UNUSED(previous_state);
            SetMpptStatus(can::MpptStatus::kStandby);
            SetDutyCycle(0.0f);
        }

        task_send_data_->ChangeInterval(EVERT_SETTING_BC_TASK_SEND_DATA_INTERVAL * 100);
    }

    void BoostConverter::ISR1_HighFrequency(const uint32_t frequency, const uint32_t cycles_max)
    {
        devices::DevKit::ISR1_HighFrequency(frequency, cycles_max);
    }

    void BoostConverter::ISR2_LowFrequency(const uint32_t frequency, const uint32_t cycles_max)
    {
        if (!bc_filters_initialized_)
        {
            // static volatile const float cutoff_frequency = 100.0f;
            // static volatile const float sampling_time = 1.0f / frequency;

            // filter_u_in_.SetAlphaBeta(cutoff_frequency, sampling_time);
            // filter_u_out_.SetAlphaBeta(cutoff_frequency, sampling_time);
            // filter_i_in_.SetAlphaBeta(cutoff_frequency, sampling_time);
            // filter_coil_temp_.SetAlphaBeta(cutoff_frequency, sampling_time);
            // filter_schottky_temp_.SetAlphaBeta(cutoff_frequency, sampling_time);
            // filter_mosfet_temp_.SetAlphaBeta(cutoff_frequency, sampling_time);

            bc_filters_initialized_ = true;
        }

        devices::DevKit::ISR2_LowFrequency(frequency, cycles_max);
    }

    void BoostConverter::ISR1_ReadSensors(const float tick)
    {
        devices::DevKit::ISR1_ReadSensors(tick);
    }

    void BoostConverter::ISR1_EnforceConstraints(const float tick)
    {
        devices::DevKit::ISR1_EnforceConstraints(tick);
    }

    void BoostConverter::ISR1_HighFrequencyControl(const float tick)
    {
        devices::DevKit::ISR1_HighFrequencyControl(tick);
    }

    void BoostConverter::ISR2_ReadSensors(const float tick)
    {
        devices::DevKit::ISR2_ReadSensors(tick);

        // Update the 'unfiltered' values
        cpu_temp_uf_ = __HAL_ADC_CALC_TEMPERATURE(EVERT_CONSTANT_DEVICE_MCU_VOLTAGE, adc1_buffer_[EVERT_SETTING_BC_ADC_RANK_CPU_TEMPERATURE], ADC_RESOLUTION_12B);
        vref_int_uf_ = __HAL_ADC_CALC_DATA_TO_VOLTAGE(EVERT_CONSTANT_DEVICE_MCU_VOLTAGE, adc1_buffer_[EVERT_SETTING_BC_ADC_RANK_VREF_INT], ADC_RESOLUTION_12B);

        // ADC values
        voltage_in_adc_ = adc1_buffer_[EVERT_SETTING_BC_ADC_RANK_VOLTAGE_IN];
        voltage_out_adc_ = adc1_buffer_[EVERT_SETTING_BC_ADC_RANK_VOLTAGE_OUT];
        current_in_adc_ = adc1_buffer_[EVERT_SETTING_BC_ADC_RANK_CURRENT_IN];

        // Update the 'filtered' values

        // Translate the ADC values to real values
        voltage_in_uf_ = maths::Lerp(voltage_in_adc_, voltage_calibration_.voltage_in_slope, voltage_calibration_.voltage_in_intercept);
        voltage_out_uf_ = maths::Lerp(voltage_out_adc_, voltage_calibration_.voltage_out_slope, voltage_calibration_.voltage_out_intercept);
        current_in_uf_ = maths::Lerp(current_in_adc_, current_calibration_.current_in_slope, current_calibration_.current_in_intercept);

        // Calculate the power in
        power_in_uf_ = voltage_in_uf_ * current_in_uf_;

        // Temperatures TODO: Readings not working
        coil_temp_uf_ = 15.0f;     // TODO: Readings not working
        schottky_temp_uf_ = 32.0f; // TODO: Readings not working
        mosfet_temp_uf_ = 45.0f;   // TODO: Readings not working

        // Apply the filters
        cpu_temp_f_ = filter_cpu_temp_.Update(cpu_temp_uf_);
        vref_int_f_ = filter_vref_int_.Update(vref_int_uf_);
        // voltage_in_f_ = EVERT_MATH_EMA(voltage_in_uf_, voltage_in_f_, 0.5f);
        // voltage_out_f_ = EVERT_MATH_EMA(voltage_out_uf_, voltage_out_f_, 0.5f);
        // current_in_f_ = EVERT_MATH_EMA(current_in_uf_, current_in_f_, 0.5f);
        voltage_in_f_ = filter_u_in_.Apply(voltage_in_uf_);
        voltage_out_f_ = filter_u_out_.Apply(voltage_out_uf_);
        current_in_f_ = filter_i_in_.Apply(current_in_uf_);
        coil_temp_f_ = filter_coil_temp_.Apply(coil_temp_uf_);
        schottky_temp_f_ = filter_schottky_temp_.Apply(schottky_temp_uf_);
        mosfet_temp_f_ = filter_mosfet_temp_.Apply(mosfet_temp_uf_);

        // Clamp above 0
        voltage_in_f_ = voltage_in_f_ > 0 ? voltage_in_f_ : 0;
        voltage_out_f_ = voltage_out_f_ > 0 ? voltage_out_f_ : 0;
        current_in_f_ = current_in_f_ > 0 ? current_in_f_ : 0;

        // Calculate the power in
        power_in_f_ = voltage_in_f_ * current_in_f_;
    }

    void BoostConverter::ISR2_EnforceConstraints(const float tick)
    {
        devices::DevKit::ISR2_EnforceConstraints(tick);

        // Only care of time-out if there is something to compare it to
        // This also helps when diagnosing/testing isolated
        if (comms_measurements_message_.last_ccu_ping != 0)
        {
            uint32_t time = HAL_GetTick();
            uint32_t delta_time = time - comms_measurements_message_.last_ccu_ping;

            if (delta_time > 1200)
            {
                RegisterDeviceAlarm(can::DeviceAlarmRegister1Index::kCriticalComms);
            }
        }

        // TODO: Id selection -- should only be done on startup, debug-purposes
        if (HAL_GPIO_ReadPin(GPIO_DEF_ID_SELECTION.port, GPIO_DEF_ID_SELECTION.pin) == GPIO_PIN_SET)
        {
            bc_dio_register_.SetBit(can::BoostConverterDioRegisterIndex::kIdSelection);
        }
        else
        {
            bc_dio_register_.ClearBit(can::BoostConverterDioRegisterIndex::kIdSelection);
        }

        // Voltage In
        UpdateBc4BandAlarms(voltage_in_f_, voltage_constraints_.voltage_in_hysteresis, voltage_constraints_.voltage_in_low_critical, voltage_constraints_.voltage_in_low_warning, voltage_constraints_.voltage_in_high_warning, voltage_constraints_.voltage_in_high_critical,
                            can::BoostConverterAlarmIndex::kUndervoltageInCritical, can::BoostConverterAlarmIndex::kUndervoltageInWarning, can::BoostConverterAlarmIndex::kOvervoltageInWarning, can::BoostConverterAlarmIndex::kOvervoltageInCritical);
        // Voltage Out
        UpdateBcHighAlarms(voltage_out_f_, voltage_constraints_.voltage_out_hysteresis, voltage_constraints_.voltage_out_high_warning, voltage_constraints_.voltage_out_high_critical,
                           can::BoostConverterAlarmIndex::kOvervoltageOutWarning, can::BoostConverterAlarmIndex::kOvervoltageOutCritical);

        // Current
        UpdateBcHighAlarms(current_in_f_, current_constraints_.current_hysteresis, current_constraints_.current_high_warning, current_constraints_.current_high_critical,
                           can::BoostConverterAlarmIndex::kOvercurrentWarning, can::BoostConverterAlarmIndex::kOvercurrentCritical);

        // Coil Temp
        UpdateBcHighAlarms(coil_temp_f_, temperature_constraints_.coil_temp_hysteresis, temperature_constraints_.coil_temp_warning, temperature_constraints_.coil_temp_critical,
                           can::BoostConverterAlarmIndex::kTemperatureCoilWarning, can::BoostConverterAlarmIndex::kTemperatureCoilCritical);

        // Schottky Temp
        UpdateBcHighAlarms(schottky_temp_f_, temperature_constraints_.schottky_temp_hysteresis, temperature_constraints_.schottky_temp_warning, temperature_constraints_.schottky_temp_critical,
                           can::BoostConverterAlarmIndex::kTemperatureSchottkyWarning, can::BoostConverterAlarmIndex::kTemperatureSchottkyCritical);

        // Mosfet Temp
        UpdateBcHighAlarms(mosfet_temp_f_, temperature_constraints_.mosfet_temp_hysteresis, temperature_constraints_.mosfet_temp_warning, temperature_constraints_.mosfet_temp_critical,
                           can::BoostConverterAlarmIndex::kTemperatureMosfetWarning, can::BoostConverterAlarmIndex::kTemperatureMosfetCritical);
    }

    void BoostConverter::ISR2_LowFrequencyControl(const float tick)
    {
        devices::DevKit::ISR2_LowFrequencyControl(tick);

        // If the device is in automatic mode, perform the perturb and observe algorithm
        if (device_mode_ == can::DeviceMode::kAutomatic)
        {
            PerformPerturbAndObserve(tick);
        }
    }

    void BoostConverter::RegisterBcAlarm(const can::BoostConverterAlarmIndex alarm_index)
    {
        if (bc_alarm_register_.SetBit(alarm_index))
        {
            ProcessInternalDeviceState();
        }
    }

    void BoostConverter::ClearBcAlarm(const can::BoostConverterAlarmIndex alarm_index)
    {
        if (bc_alarm_register_.ClearBit(alarm_index))
        {
            ProcessInternalDeviceState();
        }
    }

    void BoostConverter::ProcessInternalDeviceState()
    {
        devices::DevKit::ProcessInternalDeviceState();

        can::DeviceState device_state = GetDeviceState(DeviceStateScope::kInternal);

        if (device_state == can::DeviceState::kEmergencyShutdown || device_state == can::DeviceState::kNonOperational)
        {
            // If the device is in emergency shutdown or non-operational, return
            return;
        }

        // Get the device state by specific alarm propagation
        can::DeviceState new_state = EVERT_HAL_ALARM_BoostConverterStatePropagation(&bc_alarm_register_);
        SetDeviceState(DeviceStateScope::kInternal, new_state);
    }

    void BoostConverter::UpdateBc4BandAlarms(float value, float hysteresis, float low_critical, float low_warning, float high_warning, float high_critical,
                                             const can::BoostConverterAlarmIndex alarm_index_low_critical, const can::BoostConverterAlarmIndex alarm_index_low_warning,
                                             const can::BoostConverterAlarmIndex alarm_index_high_warning, const can::BoostConverterAlarmIndex alarm_index_high_critical)
    {
        UpdateBcLowAlarms(value, hysteresis, low_warning, low_critical, alarm_index_low_warning, alarm_index_low_critical);
        UpdateBcHighAlarms(value, hysteresis, high_warning, high_critical, alarm_index_high_warning, alarm_index_high_critical);
    }

    void BoostConverter::UpdateBcLowAlarms(float value, float hysteresis, float low_warning, float low_critical,
                                           const can::BoostConverterAlarmIndex alarm_index_low_warning, const can::BoostConverterAlarmIndex alarm_index_low_critical)
    {
        // Critical Low
        if (value < low_critical && !bc_alarm_register_.IsBitSet(alarm_index_low_critical))
        {
            RegisterBcAlarm(alarm_index_low_critical);
        }
        else if (value > low_critical + hysteresis && bc_alarm_register_.IsBitSet(alarm_index_low_critical))
        {
            ClearBcAlarm(alarm_index_low_critical);
        }

        // Warning Low
        if (value < low_warning && !bc_alarm_register_.IsBitSet(alarm_index_low_warning))
        {
            RegisterBcAlarm(alarm_index_low_warning);
        }
        else if (value > low_warning + hysteresis && bc_alarm_register_.IsBitSet(alarm_index_low_warning))
        {
            ClearBcAlarm(alarm_index_low_warning);
        }
    }

    void BoostConverter::UpdateBcHighAlarms(float value, float hysteresis, float high_warning, float high_critical,
                                            const can::BoostConverterAlarmIndex alarm_index_high_warning, const can::BoostConverterAlarmIndex alarm_index_high_critical)
    {
        // Critical High
        if (value > high_critical && !bc_alarm_register_.IsBitSet(alarm_index_high_critical))
        {
            RegisterBcAlarm(alarm_index_high_critical);
        }
        else if (value < high_critical - hysteresis && bc_alarm_register_.IsBitSet(alarm_index_high_critical))
        {
            ClearBcAlarm(alarm_index_high_critical);
        }

        // Warning High
        if (value > high_warning && !bc_alarm_register_.IsBitSet(alarm_index_high_warning))
        {
            RegisterBcAlarm(alarm_index_high_warning);
        }
        else if (value < high_warning - hysteresis && bc_alarm_register_.IsBitSet(alarm_index_high_warning))
        {
            ClearBcAlarm(alarm_index_high_warning);
        }
    }

    void BoostConverter::SetDeviceMode(can::DeviceMode device_mode)
    {
        if (device_mode == can::DeviceMode::kAutomatic && device_mode_ != can::DeviceMode::kAutomatic)
        {
            // We are entering automatic/mppt mode, set the initial duty cycle to the default value
            SetDutyCycle(0.0f);
            previous_duty_cycle_ = 0.0f;
        }

        devices::DevKit::SetDeviceMode(device_mode);
    }

    void BoostConverter::SetMpptStatus(can::MpptStatus mppt_status)
    {
        mppt_status_ = mppt_status;
    }

    void BoostConverter::SetDutyCycle(float duty_cycle)
    {
        if (duty_cycle > EVERT_CONSTRAINT_BC_DUTY_CYCLE_MAX)
        {
            duty_cycle = EVERT_CONSTRAINT_BC_DUTY_CYCLE_MAX;
        }
        else if (duty_cycle < EVERT_CONSTRAINT_BC_DUTY_CYCLE_MIN)
        {
            duty_cycle = EVERT_CONSTRAINT_BC_DUTY_CYCLE_MIN;
        }

        if (duty_cycle == duty_cycle_)
        {
            return;
        }

        HAL_HRTIM_WaveformOutputStop(hto_->hhrtim1, HRTIM_OUTPUT_TA1);
        HAL_HRTIM_WaveformCounterStop(hto_->hhrtim1, HRTIM_TIMERID_TIMER_A);

        uint32_t compare_value = period_ * duty_cycle;
        hto_->hhrtim1->Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CMP1xR = compare_value;

        HAL_HRTIM_WaveformOutputStart(hto_->hhrtim1, HRTIM_OUTPUT_TA1);
        HAL_HRTIM_WaveformCounterStart(hto_->hhrtim1, HRTIM_TIMERID_TIMER_A);

        duty_cycle_ = duty_cycle;
    }

    void BoostConverter::SetFrequency(uint32_t frequency)
    {
        if (frequency > EVERT_CONSTRAINT_BC_FREQUENCY_MAX)
        {
            frequency = EVERT_CONSTRAINT_BC_FREQUENCY_MAX;
        }
        else if (frequency < EVERT_CONSTRAINT_BC_FREQUENCY_MIN)
        {
            frequency = EVERT_CONSTRAINT_BC_FREQUENCY_MIN;
        }

        uint32_t period = EVERT_CONSTANT_DEVICE_FREQUENCY_HRCK / frequency;

        HRTIM_TimeBaseCfgTypeDef pTimeBaseCfg;
        pTimeBaseCfg.Mode = HRTIM_MODE_CONTINUOUS;
        pTimeBaseCfg.Period = period;
        pTimeBaseCfg.PrescalerRatio = HRTIM_PRESCALERRATIO_DIV1;
        pTimeBaseCfg.RepetitionCounter = 0x00;

        if (HAL_HRTIM_TimeBaseConfig(hto_->hhrtim1, HRTIM_TIMERINDEX_TIMER_A, &pTimeBaseCfg) != HAL_OK)
        {
            // TODO: Global error handling, send can messages etc
        }

        period_ = period;
        frequency_ = frequency;
    }

    void BoostConverter::SetObserveInterval(uint32_t interval)
    {
        if (interval > EVERT_CONSTRAINT_BC_OBSERVE_INTERVAL_MAX)
        {
            interval = EVERT_CONSTRAINT_BC_OBSERVE_INTERVAL_MAX;
        }
        else if (interval < EVERT_CONSTRAINT_BC_OBSERVE_INTERVAL_MIN)
        {
            interval = EVERT_CONSTRAINT_BC_OBSERVE_INTERVAL_MIN;
        }

        observe_interval_ = interval;
    }

    void BoostConverter::SetPerturbStep(float step)
    {
        if (step > EVERT_CONSTRAINT_BC_PERTURB_STEP_MAX)
        {
            step = EVERT_CONSTRAINT_BC_PERTURB_STEP_MAX;
        }
        else if (step < EVERT_CONSTRAINT_BC_PERTURB_STEP_MIN)
        {
            step = EVERT_CONSTRAINT_BC_PERTURB_STEP_MIN;
        }

        perturb_step_ = step;
    }

    void BoostConverter::SetupMessages()
    {
        devices::DevKit::SetupMessages();

        serial_handler_.Subscribe<can::AcknowledgementMessage>(std::bind(&BoostConverter::OnAcknowledgementMessageReceived, this, std::placeholders::_1, std::placeholders::_2));
        serial_handler_.Subscribe<can::DeviceConfigDevelopmentMessage>(std::bind(&BoostConverter::OnDeviceConfigDevelopmentMessageReceived, this, std::placeholders::_1, std::placeholders::_2));
        serial_handler_.Subscribe<can::PingMessage>(std::bind(&BoostConverter::OnPingMessageReceived, this, std::placeholders::_1, std::placeholders::_2));
        serial_handler_.Subscribe<can::StatePropagateMessage>(std::bind(&BoostConverter::OnStatePropagateMessageReceived, this, std::placeholders::_1, std::placeholders::_2));

        // BoostConverterConfigCurrentCalibrationMessage
        auto callbackCurrentCalibration = std::bind(&BoostConverter::OnBoostConverterConfigCurrentCalibrationMessageReceived, this, std::placeholders::_1, std::placeholders::_2);
        serial_handler_.Subscribe<can::BoostConverterConfigCurrentCalibrationMessage>(callbackCurrentCalibration);
        can_handler_.Subscribe<can::BoostConverterConfigCurrentCalibrationMessage>(callbackCurrentCalibration);

        // BoostConverterConfigCurrentConstraintsMessage
        auto callbackCurrentConstraints = std::bind(&BoostConverter::OnBoostConverterConfigCurrentConstraintsMessageReceived, this, std::placeholders::_1, std::placeholders::_2);
        serial_handler_.Subscribe<can::BoostConverterConfigCurrentConstraintsMessage>(callbackCurrentConstraints);
        can_handler_.Subscribe<can::BoostConverterConfigCurrentConstraintsMessage>(callbackCurrentConstraints);

        // BoostConverterConfigMessage
        auto callbackConfig = std::bind(&BoostConverter::OnBoostConverterConfigMessageReceived, this, std::placeholders::_1, std::placeholders::_2);
        serial_handler_.Subscribe<can::BoostConverterConfigMessage>(callbackConfig);
        can_handler_.Subscribe<can::BoostConverterConfigMessage>(callbackConfig);

        // BoostConverterConfigTemperatureConstraintsMessage
        auto callbackTemperatureConstraints = std::bind(&BoostConverter::OnBoostConverterConfigTemperatureConstraintsMessageReceived, this, std::placeholders::_1, std::placeholders::_2);
        serial_handler_.Subscribe<can::BoostConverterConfigTemperatureConstraintsMessage>(callbackTemperatureConstraints);
        can_handler_.Subscribe<can::BoostConverterConfigTemperatureConstraintsMessage>(callbackTemperatureConstraints);

        // BoostConverterConfigVoltageCalibrationMessage
        auto callbackVoltageCalibration = std::bind(&BoostConverter::OnBoostConverterConfigVoltageCalibrationMessageReceived, this, std::placeholders::_1, std::placeholders::_2);
        serial_handler_.Subscribe<can::BoostConverterConfigVoltageCalibrationMessage>(callbackVoltageCalibration);
        can_handler_.Subscribe<can::BoostConverterConfigVoltageCalibrationMessage>(callbackVoltageCalibration);

        // BoostConverterConfigVoltageConstraintsMessage
        auto callbackVoltageConstraints = std::bind(&BoostConverter::OnBoostConverterConfigVoltageConstraintsMessageReceived, this, std::placeholders::_1, std::placeholders::_2);
        serial_handler_.Subscribe<can::BoostConverterConfigVoltageConstraintsMessage>(callbackVoltageConstraints);
        can_handler_.Subscribe<can::BoostConverterConfigVoltageConstraintsMessage>(callbackVoltageConstraints);
    }

    void BoostConverter::SetupTasks()
    {
        devices::DevKit::SetupTasks();

        // TaskSendData
        task_send_data_ = std::make_shared<scheduling::Task>(std::bind(&BoostConverter::TaskSendData, this, std::placeholders::_1), EVERT_SETTING_BC_TASK_SEND_DATA_INTERVAL);
        scheduler_.AddTask(task_send_data_);
    }

    void BoostConverter::CacheStatus()
    {
        devices::DevKit::CacheStatus();

        // Update the status message
        bc_status_.dio_register = bc_dio_register_.GetRegister();
        bc_status_.alarm_register = bc_alarm_register_.GetRegister();
    }

    void BoostConverter::TaskSendData(uint32_t delta_time)
    {
        UNUSED(delta_time);

        // ADC Readings if needed
        if (device_config_development_message_.send_adc_readings)
        {
            bc_adc_readings_message_.current_in = current_in_adc_;
            bc_adc_readings_message_.voltage_in = voltage_in_adc_;
            bc_adc_readings_message_.voltage_out = voltage_out_adc_;

            can_handler_.Send(bc_adc_readings_message_);
            SendSerialMessageData(can::MessagePriority::kLow, can::MessageId::kBoostConverterAdcReadings, serializing::DataConversion::ToBytes(bc_adc_readings_message_));
        }

        // Comms Status
        can_handler_.Send(comms_measurements_message_);
        SendSerialMessageData(can::MessagePriority::kNormal, can::MessageId::kCommsMeasurements, serializing::DataConversion::ToBytes(comms_measurements_message_));

        // Measurements
        bc_measurements_message_.switching_frequency = frequency_;
        bc_measurements_message_.duty_cycle = duty_cycle_;
        bc_measurements_message_.voltage_in = voltage_in_f_;
        bc_measurements_message_.voltage_out = voltage_out_f_;
        bc_measurements_message_.current_in = current_in_f_;
        bc_measurements_message_.power_in = power_in_f_;
        bc_measurements_message_.coil_temp = coil_temp_f_;
        bc_measurements_message_.schottky_temp = schottky_temp_f_;
        bc_measurements_message_.mosfet_temp = mosfet_temp_f_;

        can_handler_.Send(bc_measurements_message_);

        std::vector<uint8_t> data = serializing::DataConversion::ToBytes(bc_measurements_message_);
        SendSerialMessageData(can::MessagePriority::kNormal, can::MessageId::kBoostConverterMeasurements, data);

        // MPPT
        bc_mppt_message_.mppt_status = mppt_status_;
        bc_mppt_message_.is_oscillating = is_oscillating_;
        bc_mppt_message_.observe_interval = observe_interval_;
        bc_mppt_message_.perturb_step = perturb_step_;
        bc_mppt_message_.previous_duty_cycle = previous_duty_cycle_;
        bc_mppt_message_.previous_power = previous_power;

        can_handler_.Send(bc_mppt_message_);

        std::vector<uint8_t> data2 = serializing::DataConversion::ToBytes(bc_mppt_message_);
        SendSerialMessageData(can::MessagePriority::kNormal, can::MessageId::kBoostConverterMppt, data2);
    }

    void BoostConverter::TaskSendAnnouncement(uint32_t delta_time)
    {
        devices::DevKit::TaskSendAnnouncement(delta_time);

        // Also send to serial
        can::AnnouncementMessage announcement_message;
        std::vector<uint8_t> message_bytes = serializing::DataConversion::ToBytes(announcement_message);
        SendSerialMessageData(can::MessagePriority::kNormal, can::MessageId::kAnnouncement, message_bytes);
    }

    void BoostConverter::TaskSendPing(uint32_t delta_time)
    {
        devices::DevKit::TaskSendPing(delta_time);

        // Also send to serial
        std::vector<uint8_t> message_bytes = serializing::DataConversion::ToBytes(ping_message_);
        SendSerialMessageData(can::MessagePriority::kNormal, can::MessageId::kPing, message_bytes);
    }

    void BoostConverter::TaskSendDeviceStatus(uint32_t delta_time)
    {
        devices::DevKit::TaskSendDeviceStatus(delta_time);
    }

    void BoostConverter::SendTotalStatus(const bool cache)
    {
        devices::DevKit::SendTotalStatus(cache);

        // Send the status object
        if (!cache || (bc_status_.dio_register != bc_status_message_.dio_register) || (bc_status_.alarm_register != bc_status_message_.alarm_register))
        {
            SendBoostConverterStatus();
        }
        else
        {
            sm_cache_saves_++;
        }
    }

    void BoostConverter::SendDeviceStatus()
    {
        devices::DevKit::SendDeviceStatus();
        SendSerialMessageData(can::MessagePriority::kLow, can::MessageId::kDeviceStatus, serializing::DataConversion::ToBytes(device_status_message_));
    }

    void BoostConverter::SendBoostConverterStatus()
    {
        bc_status_message_.dio_register = bc_status_.dio_register;
        bc_status_message_.alarm_register = bc_status_.alarm_register;
        can_handler_.Send(bc_status_message_);
        SendSerialMessageData(can::MessagePriority::kLow, can::MessageId::kBoostConverterStatus, serializing::DataConversion::ToBytes(bc_status_message_));
    }

    void BoostConverter::SendDeviceStatusAdc()
    {
        devices::DevKit::SendDeviceStatusAdc();
        SendSerialMessageData(can::MessagePriority::kLow, can::MessageId::kAdcStatus, serializing::DataConversion::ToBytes(adc_status_message_));
    }

    void BoostConverter::SendDeviceStatusCan()
    {
        devices::DevKit::SendDeviceStatusCan();
        SendSerialMessageData(can::MessagePriority::kLow, can::MessageId::kCanStatus, serializing::DataConversion::ToBytes(can_status_message_));
    }

    void BoostConverter::SendDeviceStatusClock()
    {
        devices::DevKit::SendDeviceStatusClock();
        SendSerialMessageData(can::MessagePriority::kLow, can::MessageId::kClockStatus, serializing::DataConversion::ToBytes(clock_status_message_));
    }

    void BoostConverter::SendDeviceStatusDma()
    {
        devices::DevKit::SendDeviceStatusDma();
        SendSerialMessageData(can::MessagePriority::kLow, can::MessageId::kDmaStatus, serializing::DataConversion::ToBytes(dma_status_message_));
    }

    void BoostConverter::SendDeviceStatusFlash()
    {
        devices::DevKit::SendDeviceStatusFlash();
        SendSerialMessageData(can::MessagePriority::kLow, can::MessageId::kFlashStatus, serializing::DataConversion::ToBytes(flash_status_message_));
    }

    void BoostConverter::SendDeviceStatusI2c()
    {
        devices::DevKit::SendDeviceStatusI2c();
        SendSerialMessageData(can::MessagePriority::kLow, can::MessageId::kI2cStatus, serializing::DataConversion::ToBytes(i2c_status_message_));
    }

    void BoostConverter::SendDeviceStatusUart()
    {
        devices::DevKit::SendDeviceStatusUart();
        SendSerialMessageData(can::MessagePriority::kLow, can::MessageId::kUartStatus, serializing::DataConversion::ToBytes(uart_status_message_));
    }

    void BoostConverter::SendDeviceStatusWatchdog()
    {
        devices::DevKit::SendDeviceStatusWatchdog();
        SendSerialMessageData(can::MessagePriority::kLow, can::MessageId::kWatchdogStatus, serializing::DataConversion::ToBytes(watchdog_status_message_));
    }

    void BoostConverter::PerformPerturbAndObserve(float tick)
    {
        observe_timer_ += (tick * 1000.0f);

        if (observe_timer_ > observe_interval_)
        {
            // Constraint warning, lets throttle down and see what happens
            if (mppt_status_ == can::MpptStatus::kThrottleDown)
            {
                SetDutyCycle(duty_cycle_ - perturb_step_);
            }
            else if (mppt_status_ == can::MpptStatus::kStandby)
            {
                // Standby, do nothing
                SetDutyCycle(0.0f);
            }
            else
            {
                // Observe and perturb
                Observe();
                Perturb();
            }

            observe_timer_ = 0;
        }
    }

    void BoostConverter::Observe()
    {
        float current_power = power_in_f_;
        float current_duty_cycle = duty_cycle_;

        // Are we currently cycling up or down?
        bool is_cycling_up = current_duty_cycle > previous_duty_cycle_;

        // Absolute the current perturbation size to avoid double negatives (-- = +)
        float perturb_size = current_perturb_step_ >= 0 ? perturb_step_ : -perturb_step_;

        // Is the power increasing or decreasing?
        bool power_increasing = current_power > previous_power;

        if (power_increasing)
        {
            // Power is increasing, if we are cycling up, we are on the right track, if not, we need to change direction
            current_perturb_step_ = is_cycling_up ? perturb_size : -perturb_size;

            // If the power is increasing we can use the perturbation size to determine
            // wether we are to the left or right of the MPP.
            mppt_status_ = (current_perturb_step_ > 0)
                               ? can::MpptStatus::kLeftOfMpp
                               : can::MpptStatus::kRightOfMpp;
        }
        else
        {
            // Power is decreasing, if we are cycling up; reverse, if not continue
            current_perturb_step_ = is_cycling_up ? -perturb_size : +perturb_size;

            // If the power is decreasing we can use the perturbation size to determine
            // wether we are to the left or right of the MPP.
            mppt_status_ = (current_perturb_step_ > 0)
                               ? can::MpptStatus::kLeftOfMpp
                               : can::MpptStatus::kRightOfMpp;
        }
    }

    void BoostConverter::Perturb()
    {
        float current_power = power_in_f_;
        float current_duty_cycle = duty_cycle_;
        float new_duty_cycle = current_duty_cycle + current_perturb_step_;

        // Clamp the new duty cycle to the valid range
        new_duty_cycle = std::clamp(new_duty_cycle, EVERT_CONSTRAINT_BC_DUTY_CYCLE_MPPT_MIN, EVERT_CONSTRAINT_BC_DUTY_CYCLE_MPPT_MAX);

        // Set the new duty cycle
        SetDutyCycle(new_duty_cycle);

        // Inject the data into the oscillation detector
        oscillation_detector_.injectData(current_perturb_step_);

        // Check if we are oscillating
        if (oscillation_detector_.isOscillating())
        {
            // We are oscillating, we are near the MPP
            mppt_status_ = can::MpptStatus::kNearMpp;
        }

        // Store the current state
        previous_duty_cycle_ = current_duty_cycle;
        previous_power = current_power;
    }

    void BoostConverter::SendSerialMessageData(const can::MessagePriority priority, const can::MessageId message_id, const std::vector<uint8_t> message_data)
    {
        // Can frame
        can::CanFrameData can_frame = can::CanFrameData(message_data.size(), message_data);
        std::vector<uint8_t> canFrameBytes = can::CanFrameDataConversion::ToBytes(can_frame);
        uint8_t canFrameBytesLength = canFrameBytes.size();

        // Extended identifier
        can::ExtendedIdentifier extendedIdentifier = can::ExtendedIdentifier();
        extendedIdentifier.SetSourceId(can_identifier_);
        extendedIdentifier.SetMessagePriority(priority);
        extendedIdentifier.SetMessageId(message_id);
        uint32_t identifier = extendedIdentifier.ToUint32();

        // Uart frame
        uart::UartFrameData uart_frame = uart::UartFrameData(
            uart::UartSemantics::SOH,
            identifier,
            canFrameBytesLength,
            canFrameBytes,
            uart::UartSemantics::EOT);

        std::vector<uint8_t> uartFrameBytes = uart::UartFrameDataConversion::ToBytes(uart_frame);

        serial_handler_.Transmit(uartFrameBytes);
    }

    void BoostConverter::OnAcknowledgementMessageReceived(const can::ExtendedIdentifier identifier, const can::AcknowledgementMessage &message)
    {
        if (identifier.GetTargetId() != can_identifier_)
            return;

        devices::DevKit::OnAcknowledgementMessageReceived(identifier, message);
    }

    void BoostConverter::OnDeviceConfigDevelopmentMessageReceived(const can::ExtendedIdentifier identifier, const can::DeviceConfigDevelopmentMessage &message)
    {
        // if (identifier.GetTargetId() != can_identifier_)
        //     return;

        device_config_development_message_ = message;
    }

    void BoostConverter::OnPingMessageReceived(const can::ExtendedIdentifier identifier, const can::PingMessage &message)
    {
        devices::DevKit::OnPingMessageReceived(identifier, message);
    }

    void BoostConverter::OnStatePropagateMessageReceived(const can::ExtendedIdentifier identifier, const can::StatePropagateMessage &state_propagate_message)
    {
        if (identifier.GetTargetId() != can_identifier_)
            return;

        devices::DevKit::OnStatePropagateMessageReceived(identifier, state_propagate_message);

        SetDeviceState(DeviceStateScope::kPropagated, state_propagate_message.target_state);
        // HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_10);
    }

    void BoostConverter::OnBoostConverterConfigCurrentCalibrationMessageReceived(const can::ExtendedIdentifier identifier, const can::BoostConverterConfigCurrentCalibrationMessage &message)
    {
        if (identifier.GetTargetId() != can_identifier_)
            return;

        current_calibration_ = message;
    }

    void BoostConverter::OnBoostConverterConfigCurrentConstraintsMessageReceived(const can::ExtendedIdentifier identifier, const can::BoostConverterConfigCurrentConstraintsMessage &message)
    {
        if (identifier.GetTargetId() != can_identifier_)
            return;

        current_constraints_ = message;
    }

    void BoostConverter::OnBoostConverterConfigMessageReceived(const can::ExtendedIdentifier identifier, const can::BoostConverterConfigMessage &message)
    {
        if (identifier.GetTargetId() != can_identifier_)
            return;

        SetDeviceMode(message.device_mode);
        SetFrequency(message.switching_frequency);
        SetDutyCycle(message.duty_cycle);
        SetObserveInterval(message.observe_interval);
        SetPerturbStep(message.perturb_step);
    }

    void BoostConverter::OnBoostConverterConfigTemperatureConstraintsMessageReceived(const can::ExtendedIdentifier identifier, const can::BoostConverterConfigTemperatureConstraintsMessage &message)
    {
        if (identifier.GetTargetId() != can_identifier_)
            return;

        temperature_constraints_ = message;
    }

    void BoostConverter::OnBoostConverterConfigVoltageCalibrationMessageReceived(const can::ExtendedIdentifier identifier, const can::BoostConverterConfigVoltageCalibrationMessage &message)
    {
        if (identifier.GetTargetId() != can_identifier_)
            return;

        voltage_calibration_ = message;
    }

    void BoostConverter::OnBoostConverterConfigVoltageConstraintsMessageReceived(const can::ExtendedIdentifier identifier, const can::BoostConverterConfigVoltageConstraintsMessage &message)
    {
        if (identifier.GetTargetId() != can_identifier_)
            return;

        voltage_constraints_ = message;
    }

    // UART <CAN>
    void BoostConverter::OnRawCanDataReceived(const uint32_t identifier, const uint8_t data[64])
    {
        devices::DevKit::OnRawCanDataReceived(identifier, data);

        // What we get here is the raw 64 bytes rx buffer data from the can-controller.
        // We can assert that the first byte is the length of the can frame.

        // Copy entire data to new vector buffer
        uint8_t data_length = data[0];
        uint8_t bytes_length = 1 + data_length;
        std::vector<uint8_t> can_frame_data;
        can_frame_data.reserve(bytes_length);

        for (uint8_t i = 0; i < bytes_length; i++)
        {
            can_frame_data.push_back(data[i]);
        }

        uart::UartFrameData uart_frame = uart::UartFrameData(
            uart::UartSemantics::SOH,
            identifier,
            bytes_length,
            can_frame_data,
            uart::UartSemantics::EOT);

        std::vector<uint8_t> uartFrameBytes = uart::UartFrameDataConversion::ToBytes(uart_frame);

        serial_handler_.Transmit(uartFrameBytes);
    }

    void BoostConverter::OnRawUartDataReceived(const uint32_t identifier, const std::vector<uint8_t> data)
    {
        devices::DevKit::OnRawUartDataReceived(identifier, data);

        // Data is a the raw data from an uart frame, which should be a "CanFrameData"
        //      1 byte of length
        //      n bytes of data

        can::ExtendedIdentifier extendedIdentifier = can::ExtendedIdentifier::FromIdentifier(identifier);
        can_handler_.SendRaw(extendedIdentifier, data);
    }
}
