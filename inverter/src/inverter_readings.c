#include "evert_hal_adc.h"
#include "evert_device.h"
#include "inverter.h"
#include "inverter_readings.h"

// ADC Buffers
volatile uint16_t adc1_buffer[EVERT_CONSTANT_INVERTER_ADC1_CONVERSION_COUNT] = {0};
volatile uint16_t adc2_buffer[EVERT_CONSTANT_INVERTER_ADC2_CONVERSION_COUNT] = {0};
volatile uint16_t adc3_buffer[EVERT_CONSTANT_INVERTER_ADC3_CONVERSION_COUNT] = {0};

// ADC Readings
volatile uint32_t adc_mcu_temperature = 0;
volatile uint32_t adc_mcu_vref_int = 0;
volatile uint32_t adc_current_u = 0;
volatile uint32_t adc_current_v = 0;
volatile uint32_t adc_current_w = 0;
volatile uint32_t adc_voltage_bus = 0;
volatile uint32_t adc_voltage_bus_middle = 0;
volatile uint32_t adc_voltage_u = 0;
volatile uint32_t adc_voltage_v = 0;
volatile uint32_t adc_voltage_w = 0;
volatile uint32_t adc_voltage_grid_u = 0;
volatile uint32_t adc_voltage_grid_v = 0;
volatile uint32_t adc_voltage_grid_w = 0;
volatile uint32_t adc_temperature_heatsink_u = 0;
volatile uint32_t adc_temperature_heatsink_v = 0;
volatile uint32_t adc_temperature_heatsink_w = 0;
volatile uint32_t adc_temperature_filter_coil_u = 0;
volatile uint32_t adc_temperature_filter_coil_v = 0;
volatile uint32_t adc_temperature_filter_coil_w = 0;
volatile uint32_t adc_temperature_ambient = 0;

// ADC Converted 'unfiltered' values
volatile float32_t uf_mcu_temperature = 0.0f;
volatile float32_t uf_mcu_vref_int = 0.0f;
volatile float32_t uf_bus_voltage = 0.0f;
volatile float32_t uf_bus_voltage_mid = 0.0f;
volatile float32_t uf_current_u = 0.0f;
volatile float32_t uf_current_v = 0.0f;
volatile float32_t uf_current_w = 0.0f;
volatile float32_t uf_voltage_u = 0.0f;
volatile float32_t uf_voltage_v = 0.0f;
volatile float32_t uf_voltage_w = 0.0f;
volatile float32_t uf_voltage_grid_u = 0.0f;
volatile float32_t uf_voltage_grid_v = 0.0f;
volatile float32_t uf_voltage_grid_w = 0.0f;
volatile float32_t uf_temperature_heatsink_u = 0.0f;
volatile float32_t uf_temperature_heatsink_v = 0.0f;
volatile float32_t uf_temperature_heatsink_w = 0.0f;
volatile float32_t uf_temperature_filter_coil_u = 0.0f;
volatile float32_t uf_temperature_filter_coil_v = 0.0f;
volatile float32_t uf_temperature_filter_coil_w = 0.0f;
volatile float32_t uf_temperature_ambient = 0.0f;

static float32_t fir_coeffs_32_100hz_cutoff[FILTER_TAP_NUM] = {0.004656201574947407, 0.005220870693999761, 0.006868141629260047, 0.009539625657160393, 0.013133186902443734, 0.017507053417198635, 0.02248565031717522, 0.027866910258326546, 0.033430750955371, 0.03894835727268431, 0.04419186901942923, 0.04894405662139153, 0.05300756621281193, 0.056213333467161265, 0.058427800918927825, 0.05955862508171116, 0.05955862508171116, 0.058427800918927825, 0.056213333467161265, 0.05300756621281193, 0.048944056621391514, 0.04419186901942922, 0.03894835727268431, 0.03343075095537099, 0.02786691025832654, 0.022485650317175213, 0.017507053417198635, 0.013133186902443734, 0.009539625657160393, 0.006868141629260047, 0.005220870693999761, 0.004656201574947407};

// Filters
arm_fir_instance_f32 filter_current_u;
arm_fir_instance_f32 filter_current_v;
arm_fir_instance_f32 filter_current_w;
arm_fir_instance_f32 filter_voltage_u;
arm_fir_instance_f32 filter_voltage_v;
arm_fir_instance_f32 filter_voltage_w;
arm_fir_instance_f32 filter_voltage_grid_u;
arm_fir_instance_f32 filter_voltage_grid_v;
arm_fir_instance_f32 filter_voltage_grid_w;

// Filter States
float32_t filter_current_u_state[FILTER_TAP_NUM + SAMPLE_BLOCK_SIZE - 1];
float32_t filter_current_v_state[FILTER_TAP_NUM + SAMPLE_BLOCK_SIZE - 1];
float32_t filter_current_w_state[FILTER_TAP_NUM + SAMPLE_BLOCK_SIZE - 1];
float32_t filter_voltage_u_state[FILTER_TAP_NUM + SAMPLE_BLOCK_SIZE - 1];
float32_t filter_voltage_v_state[FILTER_TAP_NUM + SAMPLE_BLOCK_SIZE - 1];
float32_t filter_voltage_w_state[FILTER_TAP_NUM + SAMPLE_BLOCK_SIZE - 1];
float32_t filter_voltage_grid_u_state[FILTER_TAP_NUM + SAMPLE_BLOCK_SIZE - 1];
float32_t filter_voltage_grid_v_state[FILTER_TAP_NUM + SAMPLE_BLOCK_SIZE - 1];
float32_t filter_voltage_grid_w_state[FILTER_TAP_NUM + SAMPLE_BLOCK_SIZE - 1];

// Filtered values
volatile float32_t fi_mcu_temperature = 0.0;
volatile float32_t fi_mcu_vref_int = 0.0;
volatile float32_t fi_bus_voltage = 0.0;
volatile float32_t fi_bus_voltage_mid = 0.0;
volatile float32_t fi_current_u = 0.0;
volatile float32_t fi_current_v = 0.0;
volatile float32_t fi_current_w = 0.0;
volatile float32_t fi_voltage_u = 0.0;
volatile float32_t fi_voltage_v = 0.0;
volatile float32_t fi_voltage_w = 0.0;
volatile float32_t fi_voltage_grid_u = 0.0;
volatile float32_t fi_voltage_grid_v = 0.0;
volatile float32_t fi_voltage_grid_w = 0.0;
volatile float32_t fi_temperature_heatsink_u = 0.0;
volatile float32_t fi_temperature_heatsink_v = 0.0;
volatile float32_t fi_temperature_heatsink_w = 0.0;
volatile float32_t fi_temperature_filter_coil_u = 0.0;
volatile float32_t fi_temperature_filter_coil_v = 0.0;
volatile float32_t fi_temperature_filter_coil_w = 0.0;
volatile float32_t fi_temperature_ambient = 0.0;

volatile bool gpio_fan_fault = false;
volatile bool gpio_pwm_fault = false;
volatile bool gpio_pwm_ready_top1 = false;
volatile bool gpio_pwm_ready_top2 = false;
volatile bool gpio_pwm_ready_top3 = false;
volatile bool gpio_pwm_ready_bot1 = false;
volatile bool gpio_pwm_ready_bot2 = false;
volatile bool gpio_pwm_ready_bot3 = false;

void EVERT_INVERTER_InitFilters(void)
{
    arm_fir_init_f32(&filter_current_u, FILTER_TAP_NUM, fir_coeffs_32_100hz_cutoff, filter_current_u_state, SAMPLE_BLOCK_SIZE);
    arm_fir_init_f32(&filter_current_v, FILTER_TAP_NUM, fir_coeffs_32_100hz_cutoff, filter_current_v_state, SAMPLE_BLOCK_SIZE);
    arm_fir_init_f32(&filter_current_w, FILTER_TAP_NUM, fir_coeffs_32_100hz_cutoff, filter_current_w_state, SAMPLE_BLOCK_SIZE);
    arm_fir_init_f32(&filter_voltage_u, FILTER_TAP_NUM, fir_coeffs_32_100hz_cutoff, filter_voltage_u_state, SAMPLE_BLOCK_SIZE);
    arm_fir_init_f32(&filter_voltage_v, FILTER_TAP_NUM, fir_coeffs_32_100hz_cutoff, filter_voltage_v_state, SAMPLE_BLOCK_SIZE);
    arm_fir_init_f32(&filter_voltage_w, FILTER_TAP_NUM, fir_coeffs_32_100hz_cutoff, filter_voltage_w_state, SAMPLE_BLOCK_SIZE);
    arm_fir_init_f32(&filter_voltage_grid_u, FILTER_TAP_NUM, fir_coeffs_32_100hz_cutoff, filter_voltage_grid_u_state, SAMPLE_BLOCK_SIZE);
    arm_fir_init_f32(&filter_voltage_grid_v, FILTER_TAP_NUM, fir_coeffs_32_100hz_cutoff, filter_voltage_grid_v_state, SAMPLE_BLOCK_SIZE);
    arm_fir_init_f32(&filter_voltage_grid_w, FILTER_TAP_NUM, fir_coeffs_32_100hz_cutoff, filter_voltage_grid_w_state, SAMPLE_BLOCK_SIZE);
}

void EVERT_INVERTER_ISR_HF_Readings(void)
{
    // ADC 1
    adc_mcu_temperature = adc1_buffer[EVERT_CONSTANT_INVERTER_ADC1_RANK_MCU_TEMPERATURE];
    adc_mcu_vref_int = adc1_buffer[EVERT_CONSTANT_INVERTER_ADC1_RANK_MCU_VREF_INT];
    adc_voltage_bus = adc1_buffer[EVERT_CONSTANT_INVERTER_ADC1_RANK_V_BUS];
    adc_voltage_bus_middle = adc1_buffer[EVERT_CONSTANT_INVERTER_ADC1_RANK_V_BUS_MID];
    adc_current_u = adc1_buffer[EVERT_CONSTANT_INVERTER_ADC1_RANK_CURRENT_U];
    adc_current_v = adc1_buffer[EVERT_CONSTANT_INVERTER_ADC1_RANK_CURRENT_V];
    adc_current_w = adc1_buffer[EVERT_CONSTANT_INVERTER_ADC1_RANK_CURRENT_W];
    adc_voltage_u = adc1_buffer[EVERT_CONSTANT_INVERTER_ADC1_RANK_VOLTAGE_U];
    adc_voltage_v = adc1_buffer[EVERT_CONSTANT_INVERTER_ADC1_RANK_VOLTAGE_V];

    // ADC 2
    adc_voltage_w = adc2_buffer[EVERT_CONSTANT_INVERTER_ADC2_RANK_VOLTAGE_W];
    adc_voltage_grid_u = adc2_buffer[EVERT_CONSTANT_INVERTER_ADC2_RANK_GRID_VOLTAGE_U];
    adc_voltage_grid_v = adc2_buffer[EVERT_CONSTANT_INVERTER_ADC2_RANK_GRID_VOLTAGE_V];
    adc_voltage_grid_w = adc2_buffer[EVERT_CONSTANT_INVERTER_ADC2_RANK_GRID_VOLTAGE_W];
    adc_temperature_heatsink_u = adc2_buffer[EVERT_CONSTANT_INVERTER_ADC2_RANK_TEMPERATURE_U];

    // Update unfiltered values
    uf_bus_voltage = EVERT_HAL_ADC_Lerp(adc_voltage_bus, calibration_voltage_bus.voltage_bus_slope, calibration_voltage_bus.voltage_bus_intercept);
    uf_bus_voltage_mid = EVERT_HAL_ADC_Lerp(adc_voltage_bus_middle, calibration_voltage_bus.voltage_bus_middle_slope, calibration_voltage_bus.voltage_bus_middle_intercept);
    uf_current_u = EVERT_HAL_ADC_Lerp(adc_current_u, calibration_current.current_u_slope, calibration_current.current_u_intercept);
    uf_current_v = EVERT_HAL_ADC_Lerp(adc_current_v, calibration_current.current_v_slope, calibration_current.current_v_intercept);
    uf_current_w = EVERT_HAL_ADC_Lerp(adc_current_w, calibration_current.current_w_slope, calibration_current.current_w_intercept);
    uf_voltage_u = EVERT_HAL_ADC_Lerp(adc_voltage_u, calibration_voltage.voltage_u_slope, calibration_voltage.voltage_u_intercept);
    uf_voltage_v = EVERT_HAL_ADC_Lerp(adc_voltage_v, calibration_voltage.voltage_v_slope, calibration_voltage.voltage_v_intercept);
    uf_voltage_w = EVERT_HAL_ADC_Lerp(adc_voltage_w, calibration_voltage.voltage_w_slope, calibration_voltage.voltage_w_intercept);
    uf_voltage_grid_u = EVERT_HAL_ADC_Lerp(adc_voltage_grid_u, calibration_voltage_grid.voltage_grid_u_slope, calibration_voltage_grid.voltage_grid_u_intercept);
    uf_voltage_grid_v = EVERT_HAL_ADC_Lerp(adc_voltage_grid_v, calibration_voltage_grid.voltage_grid_v_slope, calibration_voltage_grid.voltage_grid_v_intercept);
    uf_voltage_grid_w = EVERT_HAL_ADC_Lerp(adc_voltage_grid_w, calibration_voltage_grid.voltage_grid_w_slope, calibration_voltage_grid.voltage_grid_w_intercept);
    uf_temperature_heatsink_u = EVERT_HAL_ADC_Lerp(adc_temperature_heatsink_u, calibration_temperature.temperature_heatsink_u_slope, calibration_temperature.temperature_heatsink_u_intercept);
}

void EVERT_INVERTER_ISR_LF_Readings(void)
{
    // ADC 3
    adc_temperature_heatsink_v = adc3_buffer[EVERT_CONSTANT_INVERTER_ADC3_RANK_TEMPERATURE_V];
    adc_temperature_heatsink_w = adc3_buffer[EVERT_CONSTANT_INVERTER_ADC3_RANK_TEMPERATURE_W];
    adc_temperature_filter_coil_u = adc3_buffer[EVERT_CONSTANT_INVERTER_ADC3_RESERVE_FILTER_TEMP_U];
    adc_temperature_filter_coil_v = adc3_buffer[EVERT_CONSTANT_INVERTER_ADC3_RESERVE_FILTER_TEMP_V];
    adc_temperature_filter_coil_w = adc3_buffer[EVERT_CONSTANT_INVERTER_ADC3_RESERVE_FILTER_TEMP_W];
    adc_temperature_ambient = adc3_buffer[EVERT_CONSTANT_INVERTER_ADC3_TEMP_AMBIENT];

    // Update unfiltered values
    uf_mcu_temperature = __HAL_ADC_CALC_TEMPERATURE(EVERT_CONSTANT_DEVICE_MCU_VOLTAGE, adc_mcu_temperature, ADC_RESOLUTION_12B);
    uf_mcu_vref_int = __HAL_ADC_CALC_DATA_TO_VOLTAGE(EVERT_CONSTANT_DEVICE_MCU_VOLTAGE, adc_mcu_vref_int, ADC_RESOLUTION_12B);
    uf_temperature_heatsink_v = EVERT_HAL_ADC_Lerp(adc_temperature_heatsink_v, calibration_temperature.temperature_heatsink_v_slope, calibration_temperature.temperature_heatsink_v_intercept);
    uf_temperature_heatsink_w = EVERT_HAL_ADC_Lerp(adc_temperature_heatsink_w, calibration_temperature.temperature_heatsink_w_slope, calibration_temperature.temperature_heatsink_w_intercept);
    uf_temperature_filter_coil_u = EVERT_HAL_ADC_Lerp(adc_temperature_filter_coil_u, calibration_temperature.temperature_filter_coil_u_slope, calibration_temperature.temperature_filter_coil_u_intercept);
    uf_temperature_filter_coil_v = EVERT_HAL_ADC_Lerp(adc_temperature_filter_coil_v, calibration_temperature.temperature_filter_coil_v_slope, calibration_temperature.temperature_filter_coil_v_intercept);
    uf_temperature_filter_coil_w = EVERT_HAL_ADC_Lerp(adc_temperature_filter_coil_w, calibration_temperature.temperature_filter_coil_w_slope, calibration_temperature.temperature_filter_coil_w_intercept);
    uf_temperature_ambient = EVERT_HAL_ADC_Lerp(adc_temperature_ambient, calibration_temperature.temperature_ambient_slope, calibration_temperature.temperature_ambient_intercept);

    // Update filters
    arm_fir_f32(&filter_voltage_u, (float32_t *)&uf_voltage_u, (float32_t *)&fi_voltage_u, SAMPLE_BLOCK_SIZE);

    // Update gpio
    gpio_fan_fault = HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_0) == GPIO_PIN_RESET;
    gpio_pwm_fault = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_12) == GPIO_PIN_RESET;
    gpio_pwm_ready_top1 = HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_4) == GPIO_PIN_SET;
    gpio_pwm_ready_top2 = HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_5) == GPIO_PIN_SET;
    gpio_pwm_ready_top3 = HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_6) == GPIO_PIN_SET;
    gpio_pwm_ready_bot1 = HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_7) == GPIO_PIN_SET;
    gpio_pwm_ready_bot2 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_SET;
    gpio_pwm_ready_bot3 = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4) == GPIO_PIN_SET;
}