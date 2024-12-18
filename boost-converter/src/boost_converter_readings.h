#ifndef EVERT_BOOST_CONVERTER_READINGS_H_
#define EVERT_BOOST_CONVERTER_READINGS_H_

#include <stdbool.h>
#include <stm32g4xx_hal.h>
#include "arm_math.h"
#include "_conf_evert_boost_converter.h"
#include "boost_converter.h"

#define FIR_NUM_TAPS 51  // Number of filter taps
#define FIR_BLOCK_SIZE 1 // Process one sample at a time

// ADC Buffers
extern volatile uint16_t adc1_buffer[EVERT_CONSTANT_BC_ADC1_CONVERSION_COUNT];

// ADC Readings
extern volatile uint32_t adc_mcu_temperature;
extern volatile uint32_t adc_mcu_vref_int;
extern volatile uint32_t adc_voltage_in;
extern volatile uint32_t adc_voltage_out;
extern volatile uint32_t adc_current_in;

// ADC Converted 'unfiltered' values
extern volatile float32_t uf_mcu_temperature;
extern volatile float32_t uf_mcu_vref_int;
extern volatile float32_t uf_voltage_in;
extern volatile float32_t uf_voltage_out;
extern volatile float32_t uf_current_in;

// ADC Converted 'filtered' values
extern volatile float32_t fi_mcu_temperature;
extern volatile float32_t fi_mcu_vref_int;
extern volatile float32_t fi_voltage_in;
extern volatile float32_t fi_voltage_out;
extern volatile float32_t fi_current_in;
extern volatile float32_t fi_power_in;

void EVERT_BOOST_CONVERTER_InitFilters(void);
void EVERT_BOOST_CONVERTER_Readings(void);

#endif // EVERT_BOOST_CONVERTER_READINGS_H_