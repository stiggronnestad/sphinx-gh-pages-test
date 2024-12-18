#ifndef EVERT_INVERTER_CALIBRATION_H_
#define EVERT_INVERTER_CALIBRATION_H_

#include <arm_math.h>

typedef struct
{
    float32_t current_u_slope;
    float32_t current_u_intercept;
    float32_t current_v_slope;
    float32_t current_v_intercept;
    float32_t current_w_slope;
    float32_t current_w_intercept;
} EVERT_INVERTER_ConfigCalibrationCurrentTypeDef;

typedef struct
{
    float32_t temperature_heatsink_u_slope;
    float32_t temperature_heatsink_u_intercept;
    float32_t temperature_heatsink_v_slope;
    float32_t temperature_heatsink_v_intercept;
    float32_t temperature_heatsink_w_slope;
    float32_t temperature_heatsink_w_intercept;
    float32_t temperature_filter_coil_u_slope;
    float32_t temperature_filter_coil_u_intercept;
    float32_t temperature_filter_coil_v_slope;
    float32_t temperature_filter_coil_v_intercept;
    float32_t temperature_filter_coil_w_slope;
    float32_t temperature_filter_coil_w_intercept;
    float32_t temperature_ambient_slope;
    float32_t temperature_ambient_intercept;
} EVERT_INVERTER_ConfigCalibrationTemperatureTypeDef;

typedef struct
{
    float32_t voltage_bus_slope;
    float32_t voltage_bus_intercept;
    float32_t voltage_bus_middle_slope;
    float32_t voltage_bus_middle_intercept;
} EVERT_INVERTER_ConfigCalibrationVoltageBusTypeDef;

typedef struct
{
    float32_t voltage_grid_u_slope;
    float32_t voltage_grid_u_intercept;
    float32_t voltage_grid_v_slope;
    float32_t voltage_grid_v_intercept;
    float32_t voltage_grid_w_slope;
    float32_t voltage_grid_w_intercept;
} EVERT_INVERTER_ConfigCalibrationVoltageGridTypeDef;

typedef struct
{
    float32_t voltage_u_slope;
    float32_t voltage_u_intercept;
    float32_t voltage_v_slope;
    float32_t voltage_v_intercept;
    float32_t voltage_w_slope;
    float32_t voltage_w_intercept;
} EVERT_INVERTER_ConfigCalibrationVoltageTypeDef;

// Calibrations
extern EVERT_INVERTER_ConfigCalibrationCurrentTypeDef calibration_current;
extern EVERT_INVERTER_ConfigCalibrationTemperatureTypeDef calibration_temperature;
extern EVERT_INVERTER_ConfigCalibrationVoltageBusTypeDef calibration_voltage_bus;
extern EVERT_INVERTER_ConfigCalibrationVoltageGridTypeDef calibration_voltage_grid;
extern EVERT_INVERTER_ConfigCalibrationVoltageTypeDef calibration_voltage;

static inline void EVERT_INVERTER_InitCalibrations()
{
    calibration_current.current_u_intercept = EVERT_CALIBRATION_INV_ADC_CURRENT_U_INTERCEPT;
    calibration_current.current_u_slope = EVERT_CALIBRATION_INV_ADC_CURRENT_U_SLOPE;
    calibration_current.current_v_intercept = EVERT_CALIBRATION_INV_ADC_CURRENT_V_INTERCEPT;
    calibration_current.current_v_slope = EVERT_CALIBRATION_INV_ADC_CURRENT_V_SLOPE;
    calibration_current.current_w_intercept = EVERT_CALIBRATION_INV_ADC_CURRENT_W_INTERCEPT;

    calibration_temperature.temperature_heatsink_u_intercept = EVERT_CALIBRATION_INV_ADC_TEMPERATURE_HEATSINK_U_INTERCEPT;
    calibration_temperature.temperature_heatsink_u_slope = EVERT_CALIBRATION_INV_ADC_TEMPERATURE_HEATSINK_U_SLOPE;
    calibration_temperature.temperature_heatsink_v_intercept = EVERT_CALIBRATION_INV_ADC_TEMPERATURE_HEATSINK_V_INTERCEPT;
    calibration_temperature.temperature_heatsink_v_slope = EVERT_CALIBRATION_INV_ADC_TEMPERATURE_HEATSINK_V_SLOPE;
    calibration_temperature.temperature_heatsink_w_intercept = EVERT_CALIBRATION_INV_ADC_TEMPERATURE_HEATSINK_W_INTERCEPT;
    calibration_temperature.temperature_heatsink_w_slope = EVERT_CALIBRATION_INV_ADC_TEMPERATURE_HEATSINK_W_SLOPE;
    calibration_temperature.temperature_filter_coil_u_intercept = EVERT_CALIBRATION_INV_ADC_TEMPERATURE_FILTER_COIL_U_INTERCEPT;
    calibration_temperature.temperature_filter_coil_u_slope = EVERT_CALIBRATION_INV_ADC_TEMPERATURE_FILTER_COIL_U_SLOPE;
    calibration_temperature.temperature_filter_coil_v_intercept = EVERT_CALIBRATION_INV_ADC_TEMPERATURE_FILTER_COIL_V_INTERCEPT;
    calibration_temperature.temperature_filter_coil_v_slope = EVERT_CALIBRATION_INV_ADC_TEMPERATURE_FILTER_COIL_V_SLOPE;
    calibration_temperature.temperature_filter_coil_w_intercept = EVERT_CALIBRATION_INV_ADC_TEMPERATURE_FILTER_COIL_W_INTERCEPT;
    calibration_temperature.temperature_filter_coil_w_slope = EVERT_CALIBRATION_INV_ADC_TEMPERATURE_FILTER_COIL_W_SLOPE;
    calibration_temperature.temperature_ambient_intercept = EVERT_CALIBRATION_INV_ADC_TEMPERATURE_AMBIENT_INTERCEPT;
    calibration_temperature.temperature_ambient_slope = EVERT_CALIBRATION_INV_ADC_TEMPERATURE_AMBIENT_SLOPE;

    calibration_voltage_bus.voltage_bus_intercept = EVERT_CALIBRATION_INV_ADC_VOLTAGE_BUS_INTERCEPT;
    calibration_voltage_bus.voltage_bus_slope = EVERT_CALIBRATION_INV_ADC_VOLTAGE_BUS_SLOPE;
    calibration_voltage_bus.voltage_bus_middle_intercept = EVERT_CALIBRATION_INV_ADC_VOLTAGE_BUS_MIDDLE_INTERCEPT;
    calibration_voltage_bus.voltage_bus_middle_slope = EVERT_CALIBRATION_INV_ADC_VOLTAGE_BUS_MIDDLE_SLOPE;

    calibration_voltage_grid.voltage_grid_u_intercept = EVERT_CALIBRATION_INV_ADC_VOLTAGE_GRID_U_INTERCEPT;
    calibration_voltage_grid.voltage_grid_u_slope = EVERT_CALIBRATION_INV_ADC_VOLTAGE_GRID_U_SLOPE;
    calibration_voltage_grid.voltage_grid_v_intercept = EVERT_CALIBRATION_INV_ADC_VOLTAGE_GRID_V_INTERCEPT;
    calibration_voltage_grid.voltage_grid_v_slope = EVERT_CALIBRATION_INV_ADC_VOLTAGE_GRID_V_SLOPE;
    calibration_voltage_grid.voltage_grid_w_intercept = EVERT_CALIBRATION_INV_ADC_VOLTAGE_GRID_W_INTERCEPT;
    calibration_voltage_grid.voltage_grid_w_slope = EVERT_CALIBRATION_INV_ADC_VOLTAGE_GRID_W_SLOPE;

    calibration_voltage.voltage_u_intercept = EVERT_CALIBRATION_INV_ADC_VOLTAGE_U_INTERCEPT;
    calibration_voltage.voltage_u_slope = EVERT_CALIBRATION_INV_ADC_VOLTAGE_U_SLOPE;
    calibration_voltage.voltage_v_intercept = EVERT_CALIBRATION_INV_ADC_VOLTAGE_V_INTERCEPT;
    calibration_voltage.voltage_v_slope = EVERT_CALIBRATION_INV_ADC_VOLTAGE_V_SLOPE;
    calibration_voltage.voltage_w_intercept = EVERT_CALIBRATION_INV_ADC_VOLTAGE_W_INTERCEPT;
    calibration_voltage.voltage_w_slope = EVERT_CALIBRATION_INV_ADC_VOLTAGE_W_SLOPE;
}

#endif // EVERT_INVERTER_CALIBRATION_H_