#ifndef EVERT_INVERTER_READINGS_H_
#define EVERT_INVERTER_READINGS_H_

#include <stdbool.h>
#include <stm32g4xx_hal.h>
#include "arm_math.h"
#include "_conf_evert_inverter.h"

#define FILTER_TAP_NUM 32
#define SAMPLE_BLOCK_SIZE 10

// ADC Buffers
extern volatile uint16_t adc1_buffer[EVERT_CONSTANT_INVERTER_ADC1_CONVERSION_COUNT];
extern volatile uint16_t adc2_buffer[EVERT_CONSTANT_INVERTER_ADC2_CONVERSION_COUNT];
extern volatile uint16_t adc3_buffer[EVERT_CONSTANT_INVERTER_ADC3_CONVERSION_COUNT];

// ADC Readings
extern volatile uint32_t adc_mcu_temperature;
extern volatile uint32_t adc_mcu_vref_int;
extern volatile uint32_t adc_current_u;
extern volatile uint32_t adc_current_v;
extern volatile uint32_t adc_current_w;
extern volatile uint32_t adc_voltage_bus;
extern volatile uint32_t adc_voltage_bus_middle;
extern volatile uint32_t adc_voltage_u;
extern volatile uint32_t adc_voltage_v;
extern volatile uint32_t adc_voltage_w;
extern volatile uint32_t adc_voltage_grid_u;
extern volatile uint32_t adc_voltage_grid_v;
extern volatile uint32_t adc_voltage_grid_w;
extern volatile uint32_t adc_temperature_heatsink_u;
extern volatile uint32_t adc_temperature_heatsink_v;
extern volatile uint32_t adc_temperature_heatsink_w;
extern volatile uint32_t adc_temperature_filter_coil_u;
extern volatile uint32_t adc_temperature_filter_coil_v;
extern volatile uint32_t adc_temperature_filter_coil_w;
extern volatile uint32_t adc_temperature_ambient;

// ADC Converted 'unfiltered' values
extern volatile float32_t uf_mcu_temperature;
extern volatile float32_t uf_mcu_vref_int;
extern volatile float32_t uf_bus_voltage;
extern volatile float32_t uf_bus_voltage_mid;
extern volatile float32_t uf_current_u;
extern volatile float32_t uf_current_v;
extern volatile float32_t uf_current_w;
extern volatile float32_t uf_voltage_u;
extern volatile float32_t uf_voltage_v;
extern volatile float32_t uf_voltage_w;
extern volatile float32_t uf_voltage_grid_u;
extern volatile float32_t uf_voltage_grid_v;
extern volatile float32_t uf_voltage_grid_w;
extern volatile float32_t uf_temperature_heatsink_u;
extern volatile float32_t uf_temperature_heatsink_v;
extern volatile float32_t uf_temperature_heatsink_w;
extern volatile float32_t uf_temperature_filter_coil_u;
extern volatile float32_t uf_temperature_filter_coil_v;
extern volatile float32_t uf_temperature_filter_coil_w;
extern volatile float32_t uf_temperature_ambient;

// Filters
extern arm_fir_instance_f32 filter_current_u;
extern arm_fir_instance_f32 filter_current_v;
extern arm_fir_instance_f32 filter_current_w;
extern arm_fir_instance_f32 filter_voltage_u;
extern arm_fir_instance_f32 filter_voltage_v;
extern arm_fir_instance_f32 filter_voltage_w;
extern arm_fir_instance_f32 filter_voltage_grid_u;
extern arm_fir_instance_f32 filter_voltage_grid_v;
extern arm_fir_instance_f32 filter_voltage_grid_w;

// Filter states
extern float32_t filter_current_u_state[FILTER_TAP_NUM + SAMPLE_BLOCK_SIZE - 1];
extern float32_t filter_current_v_state[FILTER_TAP_NUM + SAMPLE_BLOCK_SIZE - 1];
extern float32_t filter_current_w_state[FILTER_TAP_NUM + SAMPLE_BLOCK_SIZE - 1];
extern float32_t filter_voltage_u_state[FILTER_TAP_NUM + SAMPLE_BLOCK_SIZE - 1];
extern float32_t filter_voltage_v_state[FILTER_TAP_NUM + SAMPLE_BLOCK_SIZE - 1];
extern float32_t filter_voltage_w_state[FILTER_TAP_NUM + SAMPLE_BLOCK_SIZE - 1];
extern float32_t filter_voltage_grid_u_state[FILTER_TAP_NUM + SAMPLE_BLOCK_SIZE - 1];
extern float32_t filter_voltage_grid_v_state[FILTER_TAP_NUM + SAMPLE_BLOCK_SIZE - 1];
extern float32_t filter_voltage_grid_w_state[FILTER_TAP_NUM + SAMPLE_BLOCK_SIZE - 1];

// Filtered values
extern volatile float32_t fi_mcu_temperature;
extern volatile float32_t fi_mcu_vref_int;
extern volatile float32_t fi_bus_voltage;
extern volatile float32_t fi_bus_voltage_mid;
extern volatile float32_t fi_current_u;
extern volatile float32_t fi_current_v;
extern volatile float32_t fi_current_w;
extern volatile float32_t fi_voltage_u;
extern volatile float32_t fi_voltage_v;
extern volatile float32_t fi_voltage_w;
extern volatile float32_t fi_voltage_grid_u;
extern volatile float32_t fi_voltage_grid_v;
extern volatile float32_t fi_voltage_grid_w;
extern volatile float32_t fi_temperature_heatsink_u;
extern volatile float32_t fi_temperature_heatsink_v;
extern volatile float32_t fi_temperature_heatsink_w;
extern volatile float32_t fi_temperature_filter_coil_u;
extern volatile float32_t fi_temperature_filter_coil_v;
extern volatile float32_t fi_temperature_filter_coil_w;
extern volatile float32_t fi_temperature_ambient;

// GPIO States
extern volatile bool gpio_fan_fault;
extern volatile bool gpio_pwm_fault;
extern volatile bool gpio_pwm_ready_top1;
extern volatile bool gpio_pwm_ready_top2;
extern volatile bool gpio_pwm_ready_top3;
extern volatile bool gpio_pwm_ready_bot1;
extern volatile bool gpio_pwm_ready_bot2;
extern volatile bool gpio_pwm_ready_bot3;

void EVERT_INVERTER_InitFilters(void);

void EVERT_INVERTER_ISR_HF_Readings(void);

void EVERT_INVERTER_ISR_LF_Readings(void);

#endif // EVERT_INVERTER_READINGS_H_