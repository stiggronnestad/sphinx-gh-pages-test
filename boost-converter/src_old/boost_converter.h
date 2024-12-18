#ifndef EVERT_BOOST_CONVERTER_H_
#define EVERT_BOOST_CONVERTER_H_

#include "boost_converter_alarm_index.h"
#include "boost_converter_alarm_state_matrix.h"
#include "boost_converter_dio_register_index.h"
#include "boost_converter_config_current_calibration_message.h"
#include "boost_converter_config_current_constraints_message.h"
#include "boost_converter_config_temperature_constraints_message.h"
#include "boost_converter_config_voltage_calibration_message.h"
#include "boost_converter_config_voltage_constraints_message.h"
#include "boost_converter_default_calibration.h"
#include "boost_converter_default_constraints.h"
#include "boost_converter_settings.h"
#include "boost_converter_status_message.h"
#include "device_hal_transfer_object.h"
#include "can_handler.h"
#include "fir_filter.h"
#include "mavg_filter.h"
#include "serial_handler.h"
#include "status_register.h"
#include "mppt_status.h"
#include "ping_message.h"
#include "oscillation_detector.h"
#include "scheduler.h"
#include "gpio_definition.h"
#include "status_led.h"
#include "device_devkit.h"

#define EVERT_MATH_EMA(new_value, prev_value, multiplier) (prev_value = ((new_value * (1.0f - multiplier)) + (prev_value * multiplier)))

namespace evert
{
    class BoostConverter : devices::DevKit
    {
    public:
        explicit BoostConverter(const can::DeviceVersionInfo device_version_info, scheduling::Scheduler &scheduler, comms::CanHandler &can_handler, comms::SerialHandler &serial_handler);
        ~BoostConverter() = default;

        static constexpr gpio::GpioDefinition GPIO_DEF_I2C_SCL = {GPIOA, GPIO_PIN_15};
        static constexpr gpio::GpioDefinition GPIO_DEF_I2C_SDA = {GPIOB, GPIO_PIN_7};
        static constexpr gpio::GpioDefinition GPIO_DEF_ID_SELECTION = {GPIOB, GPIO_PIN_5};
        static constexpr gpio::GpioDefinition GPIO_DEF_PWM = {GPIOA, GPIO_PIN_8};
        static constexpr gpio::GpioDefinition GPIO_DEF_U_IN = {GPIOB, GPIO_PIN_14}; // Voltage in
        static constexpr gpio::GpioDefinition GPIO_DEF_U_OUT = {GPIOC, GPIO_PIN_0}; // Voltage out, A5
        static constexpr gpio::GpioDefinition GPIO_DEF_I_IN = {GPIOC, GPIO_PIN_1};  // Current in, A4

        void InitHto(DEVICE_HAL_TRANSFER_OBJECT *hto) override;
        can::DeviceCanIdentifier GetCanIdentifier();
        void Loop(uint32_t tick) override;

        void ISR1_HighFrequency(const uint32_t frequency, const uint32_t cycles_max) override;
        void ISR2_LowFrequency(const uint32_t frequency, const uint32_t cycles_max) override;
        void ISR1_ReadSensors(const float tick) override;
        void ISR1_EnforceConstraints(const float tick) override;
        void ISR1_HighFrequencyControl(const float tick) override;
        void ISR2_ReadSensors(const float tick) override;
        void ISR2_EnforceConstraints(const float tick) override;
        void ISR2_LowFrequencyControl(const float tick) override;

        void RegisterBcAlarm(const can::BoostConverterAlarmIndex alarm_index);
        void ClearBcAlarm(const can::BoostConverterAlarmIndex alarm_index);
        void ProcessInternalDeviceState() override;

        using devices::Device::ClearDeviceAlarm;
        using devices::Device::GetDeviceState;
        using devices::Device::RegisterDeviceAlarm;
        using devices::Device::SetDeviceState;

        void UpdateBc4BandAlarms(float value, float hysteresis, float low_critical, float low_warning, float high_warning, float high_critical,
                                 const can::BoostConverterAlarmIndex alarm_index_low_critical, const can::BoostConverterAlarmIndex alarm_index_low_warning,
                                 const can::BoostConverterAlarmIndex alarm_index_high_warning, const can::BoostConverterAlarmIndex alarm_index_high_critical);

        void UpdateBcHighAlarms(float value, float hysteresis, float high_warning, float high_critical,
                                const can::BoostConverterAlarmIndex alarm_index_high_warning, const can::BoostConverterAlarmIndex alarm_index_high_critical);

        void UpdateBcLowAlarms(float value, float hysteresis, float low_warning, float low_critical,
                               const can::BoostConverterAlarmIndex alarm_index_low_warning, const can::BoostConverterAlarmIndex alarm_index_low_critical);

    protected:
        // Boost Converter
        void SetDeviceMode(can::DeviceMode device_mode) override;
        void SetMpptStatus(can::MpptStatus mppt_status);
        void SetDutyCycle(float duty_cycle);
        void SetFrequency(uint32_t frequency);
        void SetObserveInterval(uint32_t interval);
        void SetPerturbStep(float step);
        void PerformPerturbAndObserve(float tick);
        void Observe();
        void Perturb();

        // Device
        void SetupMessages() override;
        void SetupTasks() override;
        void CacheStatus() override;
        void TaskSendData(uint32_t deltaTime);
        void TaskSendAnnouncement(uint32_t deltaTime) override;
        void TaskSendPing(uint32_t deltaTime) override;
        void TaskSendDeviceStatus(uint32_t deltaTime) override;
        void SendTotalStatus(const bool cache) override;
        void SendDeviceStatus() override;
        void SendBoostConverterStatus();

        void SendDeviceStatusAdc() override;
        void SendDeviceStatusCan() override;
        void SendDeviceStatusClock() override;
        void SendDeviceStatusDma() override;
        void SendDeviceStatusFlash() override;
        void SendDeviceStatusI2c() override;
        void SendDeviceStatusUart() override;
        void SendDeviceStatusWatchdog() override;

        // States
        void OnEnterState_BootingComms() override;
        void OnEnterState_BootingDone() override;
        void OnEnterState_HandshakeAcknowledged(const can::DeviceState previous_state) override;
        void OnEnterState_Operational(const can::DeviceState previous_state) override;
        void OnEnterState_OperationalWarning(const can::DeviceState previous_state) override;
        void OnEnterState_NonOperational(const can::DeviceState previous_state) override;
        void OnEnterState_EmergencyShutdown(const can::DeviceState previous_state) override;

        // UART <> CAN
        void OnRawCanDataReceived(const uint32_t identifier, const uint8_t data[64]) override;
        void OnRawUartDataReceived(const uint32_t identifier, const std::vector<uint8_t> data) override;

        // Messages
        void OnAcknowledgementMessageReceived(const can::ExtendedIdentifier identifier, const can::AcknowledgementMessage &acknowledgement_message) override;
        void OnDeviceConfigDevelopmentMessageReceived(const can::ExtendedIdentifier identifier, const can::DeviceConfigDevelopmentMessage &device_config_development_message) override;
        void OnPingMessageReceived(const can::ExtendedIdentifier identifier, const can::PingMessage &ping_message) override;
        void OnStatePropagateMessageReceived(const can::ExtendedIdentifier identifier, const can::StatePropagateMessage &state_propagate_message) override;
        void OnBoostConverterConfigCurrentCalibrationMessageReceived(const can::ExtendedIdentifier identifier, const evert::protocols::can::BoostConverterConfigCurrentCalibrationMessage &message);
        void OnBoostConverterConfigCurrentConstraintsMessageReceived(const can::ExtendedIdentifier identifier, const evert::protocols::can::BoostConverterConfigCurrentConstraintsMessage &message);
        void OnBoostConverterConfigMessageReceived(const can::ExtendedIdentifier identifier, const evert::protocols::can::BoostConverterConfigMessage &message);
        void OnBoostConverterConfigTemperatureConstraintsMessageReceived(const can::ExtendedIdentifier identifier, const evert::protocols::can::BoostConverterConfigTemperatureConstraintsMessage &message);
        void OnBoostConverterConfigVoltageCalibrationMessageReceived(const can::ExtendedIdentifier identifier, const evert::protocols::can::BoostConverterConfigVoltageCalibrationMessage &message);
        void OnBoostConverterConfigVoltageConstraintsMessageReceived(const can::ExtendedIdentifier identifier, const evert::protocols::can::BoostConverterConfigVoltageConstraintsMessage &message);

    private:
        // Constraints
        can::BoostConverterConfigCurrentConstraintsMessage current_constraints_ = {EVERT_CONSTRAINT_BC_CURRENT_HYSTERESIS, EVERT_CONSTRAINT_BC_CURRENT_HIGH_CRITICAL, EVERT_CONSTRAINT_BC_CURRENT_HIGH_WARNING};
        can::BoostConverterConfigVoltageConstraintsMessage voltage_constraints_ = {EVERT_CONSTRAINT_BC_VOLTAGE_OUT_HYSTERESIS, EVERT_CONSTRAINT_BC_VOLTAGE_OUT_HIGH_CRITICAL, EVERT_CONSTRAINT_BC_VOLTAGE_OUT_HIGH_WARNING, EVERT_CONSTRAINT_BC_VOLTAGE_IN_HYSTERESIS, EVERT_CONSTRAINT_BC_VOLTAGE_IN_HIGH_CRITICAL, EVERT_CONSTRAINT_BC_VOLTAGE_IN_HIGH_WARNING, EVERT_CONSTRAINT_BC_VOLTAGE_IN_LOW_WARNING, EVERT_CONSTRAINT_BC_VOLTAGE_IN_LOW_CRITICAL};
        can::BoostConverterConfigTemperatureConstraintsMessage temperature_constraints_ = {EVERT_CONSTRAINT_BC_COIL_TEMP_HYSTERESIS, EVERT_CONSTRAINT_BC_COIL_TEMP_WARNING, EVERT_CONSTRAINT_BC_COIL_TEMP_CRITICAL, EVERT_CONSTRAINT_BC_SCHOTTKY_TEMP_HYSTERESIS, EVERT_CONSTRAINT_BC_SCHOTTKY_TEMP_WARNING, EVERT_CONSTRAINT_BC_SCHOTTKY_TEMP_CRITICAL, EVERT_CONSTRAINT_BC_MOSFET_TEMP_HYSTERESIS, EVERT_CONSTRAINT_BC_MOSFET_TEMP_WARNING, EVERT_CONSTRAINT_BC_MOSFET_TEMP_CRITICAL};

        // Config
        can::BoostConverterConfigCurrentCalibrationMessage current_calibration_ = {EVERT_CALIBRATION_BC_ADC_CURRENT_IN_SLOPE, EVERT_CALIBRATION_BC_ADC_CURRENT_IN_INTERCEPT};
        can::BoostConverterConfigVoltageCalibrationMessage voltage_calibration_ = {EVERT_CALIBRATION_BC_ADC_VOLTAGE_IN_SLOPE, EVERT_CALIBRATION_BC_ADC_VOLTAGE_IN_INTERCEPT, EVERT_CALIBRATION_BC_ADC_VOLTAGE_OUT_SLOPE, EVERT_CALIBRATION_BC_ADC_VOLTAGE_OUT_INTERCEPT};

        can::DeviceCanIdentifier can_identifier_ = can::DeviceCanIdentifier::kUnspecified;
        comms::SerialHandler &serial_handler_;
        uint32_t frequency_;
        uint32_t period_;
        float duty_cycle_;
        volatile uint16_t adc1_buffer_[EVERT_SETTING_BC_ADC1_CONVERSION_COUNT] = {0};

        // Tasks
        std::shared_ptr<scheduling::Task> task_send_data_;

        // Perturb and Observe
        can::MpptStatus mppt_status_ = can::MpptStatus::kNoData;
        OscillationDetector oscillation_detector_;
        uint32_t observe_interval_;
        float perturb_step_;         // The step size for the perturb and observe algorithm
        float current_perturb_step_; // The current step size for the perturb and observe algorithm, can be +- perturb_step_
        bool is_oscillating_ = false;
        float observe_timer_ = 0;
        float previous_duty_cycle_ = 0.5f;
        float previous_power = 0;

        // ADC Readings
        volatile float voltage_in_adc_ = 0;
        volatile float voltage_out_adc_ = 0;
        volatile float current_in_adc_ = 0;

        // Readings -- unfiltered
        volatile float voltage_in_uf_ = 0;
        volatile float voltage_out_uf_ = 0;
        volatile float current_in_uf_ = 0;
        volatile float power_in_uf_ = 0;
        volatile float coil_temp_uf_ = 0;
        volatile float schottky_temp_uf_ = 0;
        volatile float mosfet_temp_uf_ = 0;

        // Filters
        volatile bool bc_filters_initialized_ = false;
         filters::MavgFilter filter_u_in_;
         filters::MavgFilter filter_u_out_;
         filters::MavgFilter filter_i_in_;
         filters::MavgFilter filter_coil_temp_;
         filters::MavgFilter filter_schottky_temp_;
         filters::MavgFilter filter_mosfet_temp_;

        // Readings -- filtered
        volatile float voltage_in_f_ = 0;
        volatile float voltage_out_f_ = 0;
        volatile float current_in_f_ = 0;
        volatile float power_in_f_ = 0;
        volatile float coil_temp_f_ = 0;
        volatile float schottky_temp_f_ = 0;
        volatile float mosfet_temp_f_ = 0;

        // Status registers
        registers::StatusRegister<can::BoostConverterDioRegisterIndex> bc_dio_register_;
        registers::StatusRegister<can::BoostConverterAlarmIndex> bc_alarm_register_;

        // Measurements Messages - sent at a given interval
        can::BoostConverterAdcReadingsMessage bc_adc_readings_message_;  // ADC readings sent at a given interval
        can::BoostConverterMeasurementsMessage bc_measurements_message_; // Measurements sent at a given interval
        can::BoostConverterMpptMessage bc_mppt_message_;                 // MPPT status message sent at a given interval

        // Alarms/status Messages - sent when the cached status and last sent message differs
        can::BoostConverterStatusMessage bc_status_;         // Last read status from the device
        can::BoostConverterStatusMessage bc_status_message_; // Last sent status sent from the device, used for caching

        // Debug
        void SendSerialMessageData(const can::MessagePriority priority, const can::MessageId message_id, std::vector<uint8_t> data);
    };

}

#endif // EVERT_BOOST_CONVERTER_H_