#ifndef EVERT_BOOST_CONVERTER_CONSTRAINTS_H_
#define EVERT_BOOST_CONVERTER_CONSTRAINTS_H_

#include <arm_math.h>
#include "_conf_evert_boost_converter.h"

typedef struct
{
    float32_t current_hysteresis;
    float32_t current_high_critical;
    float32_t current_high_warning;
} EVERT_BOOST_CONVERTER_ConfigCurrentConstraintsTypeDef;

typedef struct
{
    float32_t coil_temp_hysteresis;
    float32_t coil_temp_critical;
    float32_t coil_temp_warning;
    float32_t schottky_temp_hysteresis;
    float32_t schottky_temp_critical;
    float32_t schottky_temp_warning;
    float32_t mosfet_temp_hysteresis;
    float32_t mosfet_temp_critical;
    float32_t mosfet_temp_warning;
} EVERT_BOOST_CONVERTER_ConfigTemperatureConstraintsTypeDef;

typedef struct
{
    float32_t voltage_out_hysteresis;
    float32_t voltage_out_high_critical;
    float32_t voltage_out_high_warning;
    float32_t voltage_in_hysteresis;
    float32_t voltage_in_high_critical;
    float32_t voltage_in_high_warning;
    float32_t voltage_in_low_warning;
    float32_t voltage_in_low_critical;
} EVERT_BOOST_CONVERTER_ConfigVoltageConstraintsTypeDef;

// Calibrations
extern EVERT_BOOST_CONVERTER_ConfigCurrentConstraintsTypeDef constraints_current;
extern EVERT_BOOST_CONVERTER_ConfigTemperatureConstraintsTypeDef constraints_temperature;
extern EVERT_BOOST_CONVERTER_ConfigVoltageConstraintsTypeDef constraints_voltage;

static inline void EVERT_BOOST_CONVERTER_InitConstraints()
{
    constraints_current.current_hysteresis = EVERT_CONSTRAINT_BC_CURRENT_HYSTERESIS;
    constraints_current.current_high_critical = EVERT_CONSTRAINT_BC_CURRENT_HIGH_CRITICAL;
    constraints_current.current_high_warning = EVERT_CONSTRAINT_BC_CURRENT_HIGH_WARNING;

    constraints_temperature.coil_temp_hysteresis = EVERT_CONSTRAINT_BC_COIL_TEMP_HYSTERESIS;
    constraints_temperature.coil_temp_critical = EVERT_CONSTRAINT_BC_COIL_TEMP_CRITICAL;
    constraints_temperature.coil_temp_warning = EVERT_CONSTRAINT_BC_COIL_TEMP_WARNING;
    constraints_temperature.schottky_temp_hysteresis = EVERT_CONSTRAINT_BC_SCHOTTKY_TEMP_HYSTERESIS;
    constraints_temperature.schottky_temp_critical = EVERT_CONSTRAINT_BC_SCHOTTKY_TEMP_CRITICAL;
    constraints_temperature.schottky_temp_warning = EVERT_CONSTRAINT_BC_SCHOTTKY_TEMP_WARNING;
    constraints_temperature.mosfet_temp_hysteresis = EVERT_CONSTRAINT_BC_MOSFET_TEMP_HYSTERESIS;
    constraints_temperature.mosfet_temp_critical = EVERT_CONSTRAINT_BC_MOSFET_TEMP_CRITICAL;
    constraints_temperature.mosfet_temp_warning = EVERT_CONSTRAINT_BC_MOSFET_TEMP_WARNING;

    constraints_voltage.voltage_out_hysteresis = EVERT_CONSTRAINT_BC_VOLTAGE_OUT_HYSTERESIS;
    constraints_voltage.voltage_out_high_critical = EVERT_CONSTRAINT_BC_VOLTAGE_OUT_HIGH_CRITICAL;
    constraints_voltage.voltage_out_high_warning = EVERT_CONSTRAINT_BC_VOLTAGE_OUT_HIGH_WARNING;
    constraints_voltage.voltage_in_hysteresis = EVERT_CONSTRAINT_BC_VOLTAGE_IN_HYSTERESIS;
    constraints_voltage.voltage_in_high_critical = EVERT_CONSTRAINT_BC_VOLTAGE_IN_HIGH_CRITICAL;
    constraints_voltage.voltage_in_high_warning = EVERT_CONSTRAINT_BC_VOLTAGE_IN_HIGH_WARNING;
    constraints_voltage.voltage_in_low_warning = EVERT_CONSTRAINT_BC_VOLTAGE_IN_LOW_WARNING;
    constraints_voltage.voltage_in_low_critical = EVERT_CONSTRAINT_BC_VOLTAGE_IN_LOW_CRITICAL;
}

#endif // EVERT_BOOST_CONVERTER_CONSTRAINTS_H_