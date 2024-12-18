#ifndef EVERT_INVERTER_CONSTRAINTS_H_
#define EVERT_INVERTER_CONSTRAINTS_H_

#include <arm_math.h>

typedef struct
{
    float32_t current_rms_hysteresis;
    float32_t current_rms_high_critical;
    float32_t current_rms_high_warning;
} EVERT_INVERTER_ConfigConstraintsCurrentTypeDef;

typedef struct
{
    float32_t voltage_bus_hysteresis;
    float32_t voltage_bus_high_critical;
    float32_t voltage_bus_high_warning;
    float32_t voltage_bus_low_warning;
    float32_t voltage_bus_low_critical;
    float32_t voltage_bus_middle_hysteresis;
    float32_t voltage_bus_middle_high_critical;
    float32_t voltage_bus_middle_high_warning;
    float32_t voltage_bus_middle_low_warning;
    float32_t voltage_bus_middle_low_critical;
} EVERT_INVERTER_ConfigConstraintsDcBusTypeDef;

typedef struct
{
    float32_t filter_coil_u_hysteresis;
    float32_t filter_coil_u_critical;
    float32_t filter_coil_u_warning;
    float32_t filter_coil_v_hysteresis;
    float32_t filter_coil_v_critical;
    float32_t filter_coil_v_warning;
    float32_t filter_coil_w_hysteresis;
    float32_t filter_coil_w_critical;
    float32_t filter_coil_w_warning;
} EVERT_INVERTER_ConfigConstraintsTemperatureFilterTypeDef;

typedef struct
{
    float32_t heatsink_u_hysteresis;
    float32_t heatsink_u_critical;
    float32_t heatsink_u_warning;
    float32_t heatsink_v_hysteresis;
    float32_t heatsink_v_critical;
    float32_t heatsink_v_warning;
    float32_t heatsink_w_hysteresis;
    float32_t heatsink_w_critical;
    float32_t heatsink_w_warning;
} EVERT_INVERTER_ConfigConstraintsTemperatureHeatsinkTypeDef;

typedef struct
{
    float32_t voltage_ln_rms_hysteresis;
    float32_t voltage_ln_rms_high_critical;
    float32_t voltage_ln_rms_high_warning;
    float32_t voltage_ln_rms_low_warning;
    float32_t voltage_ln_rms_low_critical;
    float32_t voltage_grid_ln_rms_hysteresis;
    float32_t voltage_grid_ln_rms_high_critical;
    float32_t voltage_grid_ln_rms_high_warning;
    float32_t voltage_grid_ln_rms_low_warning;
    float32_t voltage_grid_ln_rms_low_critical;
} EVERT_INVERTER_ConfigConstraintsVoltageTypeDef;

// Constraints
extern EVERT_INVERTER_ConfigConstraintsCurrentTypeDef constraints_current;
extern EVERT_INVERTER_ConfigConstraintsDcBusTypeDef constraints_dc_bus;
extern EVERT_INVERTER_ConfigConstraintsTemperatureFilterTypeDef constraints_temperature_filter;
extern EVERT_INVERTER_ConfigConstraintsTemperatureHeatsinkTypeDef constraints_temperature_heatsink;
extern EVERT_INVERTER_ConfigConstraintsVoltageTypeDef constraints_voltage;

static inline void EVERT_INVERTER_InitConstraints()
{
    constraints_current.current_rms_high_critical = EVERT_SETTING_INVERTER_CURRENT_RMS_MAX * 1.1f;
    constraints_current.current_rms_high_warning = EVERT_SETTING_INVERTER_CURRENT_RMS_MAX * 1.05f;
    constraints_current.current_rms_hysteresis = EVERT_SETTING_INVERTER_CURRENT_RMS_MAX * 0.5f;

    constraints_dc_bus.voltage_bus_high_critical = EVERT_SETTING_INVERTER_VOLTAGE_BUS_MAX * 1.1f;
    constraints_dc_bus.voltage_bus_high_warning = EVERT_SETTING_INVERTER_VOLTAGE_BUS_MAX * 1.05f;
    constraints_dc_bus.voltage_bus_low_critical = EVERT_SETTING_INVERTER_VOLTAGE_BUS_MIN * 0.9f;
    constraints_dc_bus.voltage_bus_low_warning = EVERT_SETTING_INVERTER_VOLTAGE_BUS_MIN * 0.95f;
    constraints_dc_bus.voltage_bus_middle_high_critical = EVERT_SETTING_INVERTER_VOLTAGE_BUS_MID_NOMINAL * 1.1f;
    constraints_dc_bus.voltage_bus_middle_high_warning = EVERT_SETTING_INVERTER_VOLTAGE_BUS_MID_NOMINAL * 1.05f;
    constraints_dc_bus.voltage_bus_middle_low_critical = EVERT_SETTING_INVERTER_VOLTAGE_BUS_MID_NOMINAL * 0.9f;
    constraints_dc_bus.voltage_bus_middle_low_warning = EVERT_SETTING_INVERTER_VOLTAGE_BUS_MID_NOMINAL * 0.95f;
    constraints_dc_bus.voltage_bus_middle_hysteresis = 50.0f;

    constraints_temperature_filter.filter_coil_u_critical = 100.0f;
    constraints_temperature_filter.filter_coil_u_warning = 90.0f;
    constraints_temperature_filter.filter_coil_u_hysteresis = 5.0f;
    constraints_temperature_filter.filter_coil_v_critical = 100.0f;
    constraints_temperature_filter.filter_coil_v_warning = 90.0f;
    constraints_temperature_filter.filter_coil_v_hysteresis = 5.0f;
    constraints_temperature_filter.filter_coil_w_critical = 100.0f;
    constraints_temperature_filter.filter_coil_w_warning = 90.0f;
    constraints_temperature_filter.filter_coil_w_hysteresis = 5.0f;

    constraints_temperature_heatsink.heatsink_u_critical = 100.0f;
    constraints_temperature_heatsink.heatsink_u_warning = 90.0f;
    constraints_temperature_heatsink.heatsink_u_hysteresis = 5.0f;
    constraints_temperature_heatsink.heatsink_v_critical = 100.0f;
    constraints_temperature_heatsink.heatsink_v_warning = 90.0f;
    constraints_temperature_heatsink.heatsink_v_hysteresis = 5.0f;
    constraints_temperature_heatsink.heatsink_w_critical = 100.0f;
    constraints_temperature_heatsink.heatsink_w_warning = 90.0f;
    constraints_temperature_heatsink.heatsink_w_hysteresis = 5.0f;

    constraints_voltage.voltage_grid_ln_rms_high_critical = EVERT_SETTING_INVERTER_VOLTAGE_RMS_MAX * 1.1f;
    constraints_voltage.voltage_grid_ln_rms_high_warning = EVERT_SETTING_INVERTER_VOLTAGE_RMS_MAX * 1.05f;
    constraints_voltage.voltage_grid_ln_rms_low_critical = EVERT_SETTING_INVERTER_VOLTAGE_RMS_MIN * 0.9f;
    constraints_voltage.voltage_grid_ln_rms_low_warning = EVERT_SETTING_INVERTER_VOLTAGE_RMS_MIN * 0.95f;
    constraints_voltage.voltage_ln_rms_high_critical = EVERT_SETTING_INVERTER_VOLTAGE_RMS_MAX * 1.1f;
    constraints_voltage.voltage_ln_rms_high_warning = EVERT_SETTING_INVERTER_VOLTAGE_RMS_MAX * 1.05f;
    constraints_voltage.voltage_ln_rms_low_critical = EVERT_SETTING_INVERTER_VOLTAGE_RMS_MIN * 0.9f;
    constraints_voltage.voltage_ln_rms_low_warning = EVERT_SETTING_INVERTER_VOLTAGE_RMS_MIN * 0.95f;
}

#endif // EVERT_INVERTER_CONSTRAINTS_H_