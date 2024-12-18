/**
 ******************************************************************************
 * @file    boost_converter_readings.h
 * @author  Evert Firmware Team
 * @brief   ADC readings and filtering for the Evert Boost Converter
 *
 ******************************************************************************
 **/

#include <arm_math.h>

#include "evert_hal_adc.h"
#include "evert_device.h"
#include "boost_converter_calibration.h"
#include "boost_converter_readings.h"

static float32_t FIR_COEFFS[FIR_NUM_TAPS] = {-0.000000, -0.000652, 0.001244, -0.001546, 0.001220, -0.000000, -0.002016, 0.004152, -0.005229, 0.004026, -0.000000, -0.006067, 0.011906, -0.014356, 0.010654, -0.000000, -0.015302, 0.029771, -0.036046, 0.027304, -0.000000, -0.044127, 0.097682, -0.149226, 0.186479, 0.800258, 0.186479, -0.149226, 0.097682, -0.044127, -0.000000, 0.027304, -0.036046, 0.029771, -0.015302, -0.000000, 0.010654, -0.014356, 0.011906, -0.006067, -0.000000, 0.004026, -0.005229, 0.004152, -0.002016, -0.000000, 0.001220, -0.001546, 0.001244, -0.000652, -0.000000};

// ADC Buffers
volatile uint16_t adc1_buffer[EVERT_CONSTANT_BC_ADC1_CONVERSION_COUNT] = {0};

// ADC Readings
volatile uint32_t adc_mcu_temperature = 0;
volatile uint32_t adc_mcu_vref_int = 0;
volatile uint32_t adc_voltage_in = 0;
volatile uint32_t adc_voltage_out = 0;
volatile uint32_t adc_current_in = 0;

// ADC Converted 'unfiltered' values
volatile float32_t uf_mcu_temperature = 0.0f;
volatile float32_t uf_mcu_vref_int = 0.0f;
volatile float32_t uf_voltage_in = 0.0f;
volatile float32_t uf_voltage_out = 0.0f;
volatile float32_t uf_current_in = 0.0f;

// Filters
arm_fir_instance_f32 fir_voltage_in;
arm_fir_instance_f32 fir_voltage_out;
arm_fir_instance_f32 fir_current_in;
float32_t firstate_voltage_in[FIR_NUM_TAPS + FIR_BLOCK_SIZE - 1];  // State buffer
float32_t firstate_voltage_out[FIR_NUM_TAPS + FIR_BLOCK_SIZE - 1]; // State buffer
float32_t firstate_current_in[FIR_NUM_TAPS + FIR_BLOCK_SIZE - 1];  // State buffer

// ADC Converted 'filtered' values
volatile float32_t fi_mcu_temperature;
volatile float32_t fi_mcu_vref_int;
volatile float32_t fi_voltage_in;
volatile float32_t fi_voltage_out;
volatile float32_t fi_current_in;
volatile float32_t fi_power_in;

void EVERT_BOOST_CONVERTER_InitFilters(void)
{
    arm_fir_init_f32(&fir_voltage_in, FIR_NUM_TAPS, FIR_COEFFS, firstate_voltage_in, FIR_BLOCK_SIZE);
    arm_fir_init_f32(&fir_voltage_out, FIR_NUM_TAPS, FIR_COEFFS, firstate_voltage_out, FIR_BLOCK_SIZE);
    arm_fir_init_f32(&fir_current_in, FIR_NUM_TAPS, FIR_COEFFS, firstate_current_in, FIR_BLOCK_SIZE);
}

void EVERT_BOOST_CONVERTER_Readings(void)
{
    // ADC 1
    EVERT_BOOST_CONVERTER_EMA(adc1_buffer[EVERT_CONSTANT_BC_ADC1_RANK_MCU_TEMPERATURE], adc_mcu_temperature, 0.05f);
    EVERT_BOOST_CONVERTER_EMA(adc1_buffer[EVERT_CONSTANT_BC_ADC1_RANK_MCU_VREF_INT], adc_mcu_vref_int, 0.05f);
    EVERT_BOOST_CONVERTER_EMA(adc1_buffer[EVERT_CONSTANT_BC_ADC1_RANK_VOLTAGE_IN], adc_voltage_in, 0.05f);
    EVERT_BOOST_CONVERTER_EMA(adc1_buffer[EVERT_CONSTANT_BC_ADC1_RANK_VOLTAGE_OUT], adc_voltage_out, 0.05f);
    EVERT_BOOST_CONVERTER_EMA(adc1_buffer[EVERT_CONSTANT_BC_ADC1_RANK_CURRENT_IN], adc_current_in, 0.05f);

    // Update unfiltered values
    uf_mcu_temperature = __HAL_ADC_CALC_TEMPERATURE(EVERT_CONSTANT_DEVICE_MCU_VOLTAGE, adc_mcu_temperature, ADC_RESOLUTION_12B);
    uf_mcu_vref_int = __HAL_ADC_CALC_DATA_TO_VOLTAGE(EVERT_CONSTANT_DEVICE_MCU_VOLTAGE, adc_mcu_vref_int, ADC_RESOLUTION_12B);
    uf_voltage_in = EVERT_HAL_ADC_Lerp(adc_voltage_in, calibration_voltage.voltage_in_slope, calibration_voltage.voltage_in_intercept);
    uf_voltage_out = EVERT_HAL_ADC_Lerp(adc_voltage_out, calibration_voltage.voltage_out_slope, calibration_voltage.voltage_out_intercept);
    uf_current_in = EVERT_HAL_ADC_Lerp(adc_current_in, calibration_current.current_in_slope, calibration_current.current_in_intercept);

    // Update filtered values
    arm_fir_f32(&fir_voltage_in, &uf_voltage_in, &fi_voltage_in, 1);
    arm_fir_f32(&fir_voltage_out, &uf_voltage_out, &fi_voltage_out, 1);
    arm_fir_f32(&fir_current_in, &uf_current_in, &fi_current_in, 1);

    EVERT_BOOST_CONVERTER_CLAMP(fi_voltage_in, 0.0f, 1000.0f);
    EVERT_BOOST_CONVERTER_CLAMP(fi_voltage_out, 0.0f, 1000.0f);
    EVERT_BOOST_CONVERTER_CLAMP(fi_current_in, 0.0f, 15.0f);

    fi_power_in = fi_voltage_in * fi_current_in;
}