#ifndef EVERT_BOOST_CONVERTER_CALIBRATION_H_
#define EVERT_BOOST_CONVERTER_CALIBRATION_H_

#include <arm_math.h>
#include "_conf_evert_boost_converter.h"

typedef struct
{
    float32_t current_in_slope;
    float32_t current_in_intercept;
} EVERT_BOOST_CONVERTER_ConfigCurrentCalibrationTypeDef;

typedef struct
{
    float32_t voltage_in_slope;
    float32_t voltage_in_intercept;
    float32_t voltage_out_slope;
    float32_t voltage_out_intercept;
} EVERT_BOOST_CONVERTER_ConfigVoltageCalibrationTypeDef;

// Calibrations
extern EVERT_BOOST_CONVERTER_ConfigCurrentCalibrationTypeDef calibration_current;
extern EVERT_BOOST_CONVERTER_ConfigVoltageCalibrationTypeDef calibration_voltage;

static inline void EVERT_BOOST_CONVERTER_InitCalibrations()
{
    calibration_current.current_in_slope = EVERT_CALIBRATION_BC_ADC_CURRENT_IN_SLOPE;
    calibration_current.current_in_intercept = EVERT_CALIBRATION_BC_ADC_CURRENT_IN_INTERCEPT;

    calibration_voltage.voltage_in_slope = EVERT_CALIBRATION_BC_ADC_VOLTAGE_IN_SLOPE;
    calibration_voltage.voltage_in_intercept = EVERT_CALIBRATION_BC_ADC_VOLTAGE_IN_INTERCEPT;
    calibration_voltage.voltage_out_slope = EVERT_CALIBRATION_BC_ADC_VOLTAGE_OUT_SLOPE;
    calibration_voltage.voltage_out_intercept = EVERT_CALIBRATION_BC_ADC_VOLTAGE_OUT_INTERCEPT;
}

#endif // EVERT_BOOST_CONVERTER_CALIBRATION_H_