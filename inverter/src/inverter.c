#include "stm32g4xx_hal.h"
#include "stm32g4xx_hal_hrtim.h"
#include "gpio_definition.h"
#include "inverter.h"

// Calibrations
EVERT_INVERTER_ConfigCalibrationCurrentTypeDef calibration_current;
EVERT_INVERTER_ConfigCalibrationTemperatureTypeDef calibration_temperature;
EVERT_INVERTER_ConfigCalibrationVoltageBusTypeDef calibration_voltage_bus;
EVERT_INVERTER_ConfigCalibrationVoltageGridTypeDef calibration_voltage_grid;
EVERT_INVERTER_ConfigCalibrationVoltageTypeDef calibration_voltage;

// Constraints
EVERT_INVERTER_ConfigConstraintsCurrentTypeDef constraints_current;
EVERT_INVERTER_ConfigConstraintsDcBusTypeDef constraints_dc_bus;
EVERT_INVERTER_ConfigConstraintsTemperatureFilterTypeDef constraints_temperature_filter;
EVERT_INVERTER_ConfigConstraintsTemperatureHeatsinkTypeDef constraints_temperature_heatsink;
EVERT_INVERTER_ConfigConstraintsVoltageTypeDef constraints_voltage;

EVERT_INVERTER_TimeTypeDef time;
EVERT_INVERTER_IoStateTypeDef io_state = {GPIO_PIN_RESET};

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_adc2;
extern DMA_HandleTypeDef hdma_adc3;
extern HRTIM_HandleTypeDef hhrtim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;

EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_ENABLE = {GPIOD, GPIO_PIN_1};
EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_RESET = {GPIOD, GPIO_PIN_2};
EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_READY_TOP1 = {GPIOD, GPIO_PIN_4};
EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_READY_TOP2 = {GPIOD, GPIO_PIN_5};
EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_READY_TOP3 = {GPIOD, GPIO_PIN_6};
EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_READY_BOT1 = {GPIOD, GPIO_PIN_7};
EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_READY_BOT2 = {GPIOB, GPIO_PIN_3};
EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_READY_BOT3 = {GPIOB, GPIO_PIN_4};
EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_FAULT = {GPIOA, GPIO_PIN_12};
EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_FAN_FAULT = {GPIOE, GPIO_PIN_0};
EVERT_HAL_GpioDefinitionTypeDef EVERT_INVERTER_GPIO_DEF_PWM_FDCAN_FAULT = {GPIOE, GPIO_PIN_1};

static volatile bool adc_completed[3] = {false, false, false};

//
// #region "Alarm Matrix"
//

/// @brief Device Emergency Shutdown Alarms
// static const EVERT_CAN_InverterAlarmIndexTypeDef device_emergency_shutdown_alarms[0] = {};

/// @brief Device Non-Operational Alarms
// static const EVERT_CAN_InverterAlarmIndexTypeDef device_non_operational_alarms[0] = {};

/// @brief Device Operational Warning Alarms
// static const EVERT_CAN_InverterAlarmIndexTypeDef device_operational_warning_alarms[0] = {};

//
// #endregion "Alarm Matrix"
//

int EVERT_INVERTER_main()
{
    HAL_DMA_RegisterCallback(&hdma_adc1, HAL_DMA_XFER_ERROR_CB_ID, HAL_DMA_ErrorCallback);
    HAL_DMA_RegisterCallback(&hdma_adc2, HAL_DMA_XFER_ERROR_CB_ID, HAL_DMA_ErrorCallback);
    HAL_DMA_RegisterCallback(&hdma_adc3, HAL_DMA_XFER_ERROR_CB_ID, HAL_DMA_ErrorCallback);

    EVERT_INVERTER_InitCalibrations();
    EVERT_INVERTER_InitConstraints();
    EVERT_DEVICE_SetVersionInfo(DEVICE_VERSION_MAJOR, DEVICE_VERSION_MINOR, DEVICE_VERSION_PATCH);

    // Initialize the time
    time.start_time = HAL_GetTick();

    // Setup CORDIC
    LL_CORDIC_Config(CORDIC,
                     LL_CORDIC_FUNCTION_COSINE,   /* cosine function */
                     LL_CORDIC_PRECISION_6CYCLES, /* max precision for q1.31 cosine */
                     LL_CORDIC_SCALE_0,           /* no scale */
                     LL_CORDIC_NBWRITE_1,         /* One input data: angle. Second input data (modulus) is 1 after cordic reset */
                     LL_CORDIC_NBREAD_2,          /* Two output data: cosine, then sine */
                     LL_CORDIC_INSIZE_32BITS,     /* q1.31 format for input data */
                     LL_CORDIC_OUTSIZE_32BITS);   /* q1.31 format for output data */

    // Setup the filters
    EVERT_INVERTER_InitFilters();

    // Initialize the device
    EVERT_DEVICE_Init();

    // Start the ADCs
    EVERT_HAL_ADC_Start(&hadc1, (uint16_t *)adc1_buffer, EVERT_CONSTANT_INVERTER_ADC1_CONVERSION_COUNT);
    EVERT_HAL_ADC_Start(&hadc2, (uint16_t *)adc2_buffer, EVERT_CONSTANT_INVERTER_ADC2_CONVERSION_COUNT);
    EVERT_HAL_ADC_Start(&hadc3, (uint16_t *)adc3_buffer, EVERT_CONSTANT_INVERTER_ADC3_CONVERSION_COUNT);

    // Start the timers
    HAL_TIM_Base_Start_IT(&htim3); // 100 Hz (LF ISR)
    HAL_TIM_Base_Start_IT(&htim6); // 25 kHz (HF ISR)

    // Setup the HRTIM/PWM
    EVERT_INVERTER_SetDeadtime(HRTIM_TIMERINDEX_TIMER_A, 50);
    EVERT_INVERTER_SetDeadtime(HRTIM_TIMERINDEX_TIMER_B, 50);
    EVERT_INVERTER_SetDeadtime(HRTIM_TIMERINDEX_TIMER_C, 50);
    EVERT_INVERTER_SetDeadtime(HRTIM_TIMERINDEX_TIMER_D, 50);
    EVERT_INVERTER_SetDeadtime(HRTIM_TIMERINDEX_TIMER_E, 50);
    EVERT_INVERTER_SetDeadtime(HRTIM_TIMERINDEX_TIMER_F, 50);
    EVERT_INVERTER_SetPwmEnabled(true);
    // EVERT_INVERTER_SetDutyCycle(0.5f, 0.25f, -0.25f);

    // Setup fans
    // EVERT_EMC230X_Init();
    // EVERT_EMC230X_SetAllFanSpeeds_IT(0);
    // HAL_Delay(10);
    // EVERT_EMC230X_ReadTachs_IT();

    // LEDS RGB
    EVERT_FZ2812_Init(&htim2);
    EVERT_FZ2812_SetBlink(0, 1000, 1000);

    // Grid Forming
    static const float32_t kp = 150;
    static const float32_t ki = 11447;
    static const float32_t coeff_b0 = 150.0155528;
    static const float32_t coeff_b1 = -149.5576746;
    // EVERT_INVERTER_GridFormingInit(kp, ki, coeff_b0, coeff_b1);

    return 0;
}

void EVERT_INVERTER_loop()
{
    time.current_time = HAL_GetTick();
    time.elapsed_time = time.current_time - time.start_time;
    time.delta_time = time.current_time - time.last_time;

    EVERT_DEVICE_Update(time.elapsed_time, time.delta_time);
    EVERT_FZ2812_Update();

    time.last_time = time.current_time;
}

#define TWO_PI 6.28318530718f        // 2 * pi
#define PHASE_OFFSET (TWO_PI / 3.0f) // 120 degrees in radians
#define Q31_MULTIPLIER 2147483648.0f // For converting to Q1.31 fixed-point

void GetDutyCycles(float32_t *duty_u, float32_t *duty_v, float32_t *duty_w)
{
    static float32_t current_angle = 0.0f;                        // Persistent angle
    const float32_t omega_t = TWO_PI * 50.2f * (1.0f / 25000.0f); // 0.1 Hz, 25 kHz sampling rate

    // Update angle and wrap within [0, 2*pi)
    current_angle += omega_t;
    if (current_angle >= TWO_PI)
    {
        current_angle -= TWO_PI; // Wrap-around
    }

    // Calculate phase angles
    float32_t angle0 = current_angle;
    float32_t angle120 = fmodf(angle0 + PHASE_OFFSET, TWO_PI);
    float32_t angle240 = fmodf(angle0 + 2 * PHASE_OFFSET, TWO_PI);

    float32_t sin_u = sinf(angle0);
    float32_t sin_v = sinf(angle120);
    float32_t sin_w = sinf(angle240);

    *duty_u = sin_u;
    *duty_v = sin_v;
    *duty_w = sin_w;

    // // Convert angles to Q1.31 format
    // int32_t q31_angle0 = (int32_t)(angle0 * (Q31_MULTIPLIER / TWO_PI));
    // int32_t q31_angle120 = (int32_t)(angle120 * (Q31_MULTIPLIER / TWO_PI));
    // int32_t q31_angle240 = (int32_t)(angle240 * (Q31_MULTIPLIER / TWO_PI));

    // // Write angle to CORDIC and read sine and cosine values
    // LL_CORDIC_WriteData(CORDIC, q31_angle0); // Phase U
    // float32_t sin_u = (float32_t)((int32_t)LL_CORDIC_ReadData(CORDIC)) / Q31_MULTIPLIER;
    // float32_t cos_u = (float32_t)((int32_t)LL_CORDIC_ReadData(CORDIC)) / Q31_MULTIPLIER;
    // *duty_u = sin_u * sin_u * ((sin_u >= 0) ? 1.0f : -1.0f);

    // LL_CORDIC_WriteData(CORDIC, q31_angle120); // Phase V
    // float32_t sin_v = (float32_t)((int32_t)LL_CORDIC_ReadData(CORDIC)) / Q31_MULTIPLIER;
    // float32_t cos_v = (float32_t)((int32_t)LL_CORDIC_ReadData(CORDIC)) / Q31_MULTIPLIER;
    // *duty_v = sin_v;

    // LL_CORDIC_WriteData(CORDIC, q31_angle240); // Phase W
    // float32_t sin_w = (float32_t)((int32_t)LL_CORDIC_ReadData(CORDIC)) / Q31_MULTIPLIER;
    // float32_t cos_w = (float32_t)((int32_t)LL_CORDIC_ReadData(CORDIC)) / Q31_MULTIPLIER;
    // *duty_w = sin_w;

    // Debugging values
    // gf_cosine = cos_u; // Replace with cosine read if needed
    gf_duty_cycle_a_pu = *duty_u;
    gf_duty_cycle_b_pu = *duty_v;
    gf_duty_cycle_c_pu = *duty_w;
}

float32_t EVERT_INVERTER_GetDutyCycleCcrByPercentage(const float32_t percentage)
{
    uint32_t compare = (uint32_t)EVERT_CONSTANT_INVERTER_PWM_PERIOD - (percentage * EVERT_CONSTANT_INVERTER_PWM_PERIOD);

    if (compare < 96)
        compare = 96;

    if (compare > (EVERT_CONSTANT_INVERTER_PWM_PERIOD))
        compare = (EVERT_CONSTANT_INVERTER_PWM_PERIOD);

    return compare;
}

void EVERT_INVERTER_SetDeadtime(const uint32_t timer_index, uint16_t deadtime)
{
    static const uint16_t DEAD_TIME_MIN = 10;
    static const uint16_t DEAD_TIME_MAX = 511;

    if (deadtime < DEAD_TIME_MIN)
        deadtime = DEAD_TIME_MIN;

    if (deadtime > DEAD_TIME_MAX)
        deadtime = DEAD_TIME_MAX;

    HRTIM_DeadTimeCfgTypeDef pDeadTimeCfg;
    pDeadTimeCfg.Prescaler = HRTIM_TIMDEADTIME_PRESCALERRATIO_MUL8;
    pDeadTimeCfg.RisingValue = deadtime;
    pDeadTimeCfg.RisingSign = HRTIM_TIMDEADTIME_RISINGSIGN_POSITIVE;
    pDeadTimeCfg.RisingLock = HRTIM_TIMDEADTIME_RISINGLOCK_WRITE;
    pDeadTimeCfg.RisingSignLock = HRTIM_TIMDEADTIME_RISINGSIGNLOCK_WRITE;
    pDeadTimeCfg.FallingValue = deadtime;
    pDeadTimeCfg.FallingSign = HRTIM_TIMDEADTIME_FALLINGSIGN_POSITIVE;
    pDeadTimeCfg.FallingLock = HRTIM_TIMDEADTIME_FALLINGLOCK_WRITE;
    pDeadTimeCfg.FallingSignLock = HRTIM_TIMDEADTIME_FALLINGSIGNLOCK_WRITE;

    if (HAL_HRTIM_DeadTimeConfig(&hhrtim1, timer_index, &pDeadTimeCfg) != HAL_OK)
    {
        EVERT_INVERTER_hal_error("Error setting deadtime\n");
    }
}

void EVERT_INVERTER_SetDutyCycle(const float32_t duty_cycle_u, const float32_t duty_cycle_v, const float32_t duty_cycle_w)
{
    // Max period referenced TIMER_A, all timers have the same period
    // const uint32_t period = HRTIM1->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].PERxR;
    const uint32_t period = 27200;

    // Calculate absolute duty cycles
    float32_t duty_cycleu_abs;
    float32_t duty_cyclev_abs;
    float32_t duty_cyclew_abs;

    arm_abs_f32(&duty_cycle_u, &duty_cycleu_abs, 1);
    arm_abs_f32(&duty_cycle_v, &duty_cyclev_abs, 1);
    arm_abs_f32(&duty_cycle_w, &duty_cyclew_abs, 1);

    // Calculate compares, set/reset triggers around midpoint
    uint32_t compare_u = period - ((uint32_t)(period * duty_cycleu_abs));
    uint32_t compare_v = period - ((uint32_t)(period * duty_cyclev_abs));
    uint32_t compare_w = period - ((uint32_t)(period * duty_cyclew_abs));

    static const uint32_t MIN_COMPARE = 70;
    static const uint32_t MAX_COMPARE = 27134;

    if (compare_u < MIN_COMPARE)
        compare_u = MIN_COMPARE;

    if (compare_v < MIN_COMPARE)
        compare_v = MIN_COMPARE;

    if (compare_w < MIN_COMPARE)
        compare_w = MIN_COMPARE;

    if (compare_u > MAX_COMPARE)
        compare_u = MAX_COMPARE;

    if (compare_v > MAX_COMPARE)
        compare_v = MAX_COMPARE;

    if (compare_w > MAX_COMPARE)
        compare_w = MAX_COMPARE;

    static bool is_upper_pair_enabled_u = false;
    static bool is_lower_pair_enabled_u = false;
    static bool is_upper_pair_enabled_v = false;
    static bool is_lower_pair_enabled_v = false;
    static bool is_upper_pair_enabled_w = false;
    static bool is_lower_pair_enabled_w = false;

    static const float32_t MID_DELTA = 0.0000000001f;

    if (duty_cycle_u < MID_DELTA && is_upper_pair_enabled_u)
    {
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, 1);
        hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CNTxR = 0;
        is_upper_pair_enabled_u = false;
    }

    if (duty_cycle_u > MID_DELTA && is_lower_pair_enabled_u)
    {
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1, 1);
        hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CNTxR = 0;
        is_lower_pair_enabled_u = false;
    }

    if (duty_cycle_v < MID_DELTA && is_upper_pair_enabled_v)
    {
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_1, 1);
        hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CNTxR = 0;
        is_upper_pair_enabled_v = false;
    }

    if (duty_cycle_v > MID_DELTA && is_lower_pair_enabled_v)
    {
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_1, 1);
        hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CNTxR = 0;
        is_lower_pair_enabled_v = false;
    }

    if (duty_cycle_w < MID_DELTA && is_upper_pair_enabled_w)
    {
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_E, HRTIM_COMPAREUNIT_1, 1);
        hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_E].CNTxR = 0;
        is_upper_pair_enabled_w = false;
    }

    if (duty_cycle_w > MID_DELTA && is_lower_pair_enabled_w)
    {
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_F, HRTIM_COMPAREUNIT_1, 1);
        hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_F].CNTxR = 0;
        is_lower_pair_enabled_w = false;
    }

    if (is_upper_pair_enabled_u)
    {
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_A, HRTIM_COMPAREUNIT_1, compare_u);
    }

    if (is_lower_pair_enabled_u)
    {
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_B, HRTIM_COMPAREUNIT_1, compare_u);
    }

    if (is_upper_pair_enabled_v)
    {
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_C, HRTIM_COMPAREUNIT_1, compare_v);
    }

    if (is_lower_pair_enabled_v)
    {
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_D, HRTIM_COMPAREUNIT_1, compare_v);
    }

    if (is_upper_pair_enabled_w)
    {
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_E, HRTIM_COMPAREUNIT_1, compare_w);
    }

    if (is_lower_pair_enabled_w)
    {
        __HAL_HRTIM_SETCOMPARE(&hhrtim1, HRTIM_TIMERINDEX_TIMER_F, HRTIM_COMPAREUNIT_1, compare_w);
    }

    if (duty_cycle_u > 0.0000000001f && !is_upper_pair_enabled_u)
    {
        hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CNTxR = 0;
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
        is_upper_pair_enabled_u = true;
    }

    if (duty_cycle_u < -0.0000000001f && !is_lower_pair_enabled_u)
    {
        hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CNTxR = 0;
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);
        is_lower_pair_enabled_u = true;
    }

    if (duty_cycle_v > 0.0000000001f && !is_upper_pair_enabled_v)
    {
        hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CNTxR = 0;
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TC2);
        is_upper_pair_enabled_v = true;
    }

    if (duty_cycle_v < -0.0000000001f && !is_lower_pair_enabled_v)
    {
        hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CNTxR = 0;
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
        is_lower_pair_enabled_v = true;
    }

    if (duty_cycle_w > 0.0000000001f && !is_upper_pair_enabled_w)
    {
        hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_E].CNTxR = 0;
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TE1 | HRTIM_OUTPUT_TE2);
        is_upper_pair_enabled_w = true;
    }

    if (duty_cycle_w < -0.0000000001f && !is_lower_pair_enabled_w)
    {
        hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_F].CNTxR = 0;
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2);
        is_lower_pair_enabled_w = true;
    }

    // if (duty_cycle_w < -0.5f && !is_lower_pair_enabled_w)
    // {
    //     HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TE1 | HRTIM_OUTPUT_TE2);
    //     is_upper_pair_enabled_w = false;
    //     hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_F].CNTxR = 0;
    //     HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2);
    //     is_lower_pair_enabled_w = true;
    //     w_lower_pair_on++;
    // }

    // if (duty_cycle_u > 0.0f)
    // {
    //     // Disable TB1 & TB2 for PWM B (Lower Pair)
    //     HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);

    //     // Reset TIMER_A counter
    //     hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_A].CNTxR = 0;

    //     // Enable TA1 & TA2 for PWM A (Upper Pair)
    //     HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
    // }
    // else
    // {
    //     // Disable TA1 & TA2 for PWM A (Upper Pair)
    //     HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);

    //     // Reset TIMER_B counter
    //     hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_B].CNTxR = 0;

    //     // Enable TB1 & TB2 for PWM B (Lower Pair)
    //     HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);
    // }

    // if (duty_cycle_v > 0.0f)
    // {
    //     // Disable TD1 & TD2 for PWM D (Lower Pair)
    //     HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);

    //     // Reset TIMER_D counter
    //     hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_C].CNTxR = 0;

    //     // Enable TC1 & TC2 for PWM C (Upper Pair)
    //     HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TC2);
    // }
    // else
    // {
    //     // Disable TC1 & TC2 for PWM C (Upper Pair)
    //     HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TC2);

    //     // Reset TIMER_C counter
    //     hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_D].CNTxR = 0;

    //     // Enable TD1 & TD2 for PWM D (Lower Pair)
    //     HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
    // }

    // if (duty_cycle_w > 0.0f)
    // {
    //     // Disable TF1 & TF2 for PWM F (Lower Pair)
    //     HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2);

    //     // Reset TIMER_F counter
    //     hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_E].CNTxR = 0;

    //     // Enable TE1 & TE2 for PWM E (Upper Pair)
    //     HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TE1 | HRTIM_OUTPUT_TE2);
    // }
    // else
    // {
    //     // Disable TE1 & TE2 for PWM E (Upper Pair)
    //     HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TE1 | HRTIM_OUTPUT_TE2);

    //     // Reset TIMER_E counter
    //     hhrtim1.Instance->sTimerxRegs[HRTIM_TIMERINDEX_TIMER_F].CNTxR = 0;

    //     // Enable TF1 & TF2 for PWM F (Lower Pair)
    //     HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2);
    // }
}

void EVERT_INVERTER_SetPwmEnabled(const bool enabled)
{
    if (enabled)
    {
        // Gate driver enable - active high
        HAL_GPIO_WritePin(EVERT_INVERTER_GPIO_DEF_PWM_ENABLE.port, EVERT_INVERTER_GPIO_DEF_PWM_ENABLE.pin, GPIO_PIN_SET);

        // Gate driver reset/enable - reset low, enable high
        HAL_GPIO_WritePin(EVERT_INVERTER_GPIO_DEF_PWM_RESET.port, EVERT_INVERTER_GPIO_DEF_PWM_RESET.pin, GPIO_PIN_SET);

        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
        HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_TIMER_A);
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);
        HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_TIMER_B);
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TC2);
        HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_TIMER_C);
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
        HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_TIMER_D);
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TE1 | HRTIM_OUTPUT_TE2);
        HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_TIMER_E);
        HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2);
        HAL_HRTIM_WaveformCounterStart(&hhrtim1, HRTIM_TIMERID_TIMER_F);
    }
    else
    {
        // Gate driver enable - active high
        HAL_GPIO_WritePin(EVERT_INVERTER_GPIO_DEF_PWM_ENABLE.port, EVERT_INVERTER_GPIO_DEF_PWM_ENABLE.pin, GPIO_PIN_RESET);

        // Gate driver reset/enable - reset low, enable high
        HAL_GPIO_WritePin(EVERT_INVERTER_GPIO_DEF_PWM_RESET.port, EVERT_INVERTER_GPIO_DEF_PWM_RESET.pin, GPIO_PIN_RESET);

        HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TA1 | HRTIM_OUTPUT_TA2);
        HAL_HRTIM_WaveformCounterStop(&hhrtim1, HRTIM_TIMERID_TIMER_A);
        HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TB1 | HRTIM_OUTPUT_TB2);
        HAL_HRTIM_WaveformCounterStop(&hhrtim1, HRTIM_TIMERID_TIMER_B);
        HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TC1 | HRTIM_OUTPUT_TC2);
        HAL_HRTIM_WaveformCounterStop(&hhrtim1, HRTIM_TIMERID_TIMER_C);
        HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TD1 | HRTIM_OUTPUT_TD2);
        HAL_HRTIM_WaveformCounterStop(&hhrtim1, HRTIM_TIMERID_TIMER_D);
        HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TE1 | HRTIM_OUTPUT_TE2);
        HAL_HRTIM_WaveformCounterStop(&hhrtim1, HRTIM_TIMERID_TIMER_E);
        HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TF1 | HRTIM_OUTPUT_TF2);
        HAL_HRTIM_WaveformCounterStop(&hhrtim1, HRTIM_TIMERID_TIMER_F);
    }
}

void EVERT_INVERTER_SetStatusLed(const uint8_t index, const uint32_t color, const uint32_t time_on, const uint32_t time_off)
{
    EVERT_FZ2812_SetColor(index, color);
    EVERT_FZ2812_SetBlink(index, time_on, time_off);
}

// __weak Callbacks - State
void __overrides EVERT_DEVICE_Derived_OnDeviceStateChange(const EVERT_DEVICE_StateTypeDef new_state, const EVERT_DEVICE_StateTypeDef old_state)
{
    UNUSED(new_state);
    UNUSED(old_state);

    switch (new_state)
    {
    case DS_BOOTING:
        EVERT_INVERTER_SetStatusLed(0, EVERT_CONSTANT_INVERTER_STATUS_LED0_COLOR_BOOTING, EVERT_CONSTANT_INVERTER_STATUS_LED0_TIME_BOOTING_ON, EVERT_CONSTANT_INVERTER_STATUS_LED0_TIME_BOOTING_OFF);
        EVERT_INVERTER_SetStatusLed(1, EVERT_CONSTANT_INVERTER_STATUS_LED1_COLOR_BOOTING, EVERT_CONSTANT_INVERTER_STATUS_LED1_TIME_BOOTING_ON, EVERT_CONSTANT_INVERTER_STATUS_LED1_TIME_BOOTING_OFF);
        break;

    case DS_OPERATIONAL:
        EVERT_INVERTER_SetStatusLed(0, EVERT_CONSTANT_INVERTER_STATUS_LED0_COLOR_OK, EVERT_CONSTANT_INVERTER_STATUS_LED0_TIME_OK_ON, EVERT_CONSTANT_INVERTER_STATUS_LED0_TIME_OK_OFF);
        EVERT_INVERTER_SetStatusLed(1, EVERT_CONSTANT_INVERTER_STATUS_LED1_COLOR_OK, EVERT_CONSTANT_INVERTER_STATUS_LED1_TIME_OK_ON, EVERT_CONSTANT_INVERTER_STATUS_LED1_TIME_OK_OFF);
        break;

    case DS_OPERATIONAL_WARNING:
        EVERT_INVERTER_SetStatusLed(0, EVERT_CONSTANT_INVERTER_STATUS_LED0_COLOR_WARNING, EVERT_CONSTANT_INVERTER_STATUS_LED0_TIME_WARNING_ON, EVERT_CONSTANT_INVERTER_STATUS_LED0_TIME_WARNING_OFF);
        EVERT_INVERTER_SetStatusLed(1, EVERT_CONSTANT_INVERTER_STATUS_LED1_COLOR_WARNING, EVERT_CONSTANT_INVERTER_STATUS_LED1_TIME_WARNING_ON, EVERT_CONSTANT_INVERTER_STATUS_LED1_TIME_WARNING_OFF);
        break;

    case DS_EMERGENCY_SHUTDOWN:
        EVERT_INVERTER_SetStatusLed(0, EVERT_CONSTANT_INVERTER_STATUS_LED0_COLOR_ERROR, EVERT_CONSTANT_INVERTER_STATUS_LED0_TIME_ERROR_ON, EVERT_CONSTANT_INVERTER_STATUS_LED0_TIME_ERROR_OFF);
        EVERT_INVERTER_SetStatusLed(1, EVERT_CONSTANT_INVERTER_STATUS_LED1_COLOR_ERROR, EVERT_CONSTANT_INVERTER_STATUS_LED1_TIME_ERROR_ON, EVERT_CONSTANT_INVERTER_STATUS_LED1_TIME_ERROR_OFF);
        break;

    default:
        break;
    }
}

void __overrides EVERT_DEVICE_Derived_OnEnterState_BootingComms()
{
    EVERT_DEVICE_State_Set(SS_INTERNAL, DS_BOOTING_DONE);
}

// __weak Callbacks - Task Scheduler
void __overrides EVERT_TASK_SCHEDULER_OnTaskSendAnnouncement(void)
{
}
void __overrides EVERT_TASK_SCHEDULER_OnTaskSendData(void)
{
}
void __overrides EVERT_TASK_SCHEDULER_OnTaskSendDeviceStatus(void)
{
}
void __overrides EVERT_TASK_SCHEDULER_OnTaskSendPing(void)
{
}

// Peripheral Callbacks
void EVERT_INVERTER_ISR_25KHZ_IRQHandler()
{
    // Verify the ISR frequency
    // HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_10);

    if (!adc_completed[0] || !adc_completed[1])
    {
        return;
    }

    io_state.pwm_ready_top1 = HAL_GPIO_ReadPin(EVERT_INVERTER_GPIO_DEF_PWM_READY_TOP1.port, EVERT_INVERTER_GPIO_DEF_PWM_READY_TOP1.pin);
    io_state.pwm_ready_top2 = HAL_GPIO_ReadPin(EVERT_INVERTER_GPIO_DEF_PWM_READY_TOP2.port, EVERT_INVERTER_GPIO_DEF_PWM_READY_TOP2.pin);
    io_state.pwm_ready_top3 = HAL_GPIO_ReadPin(EVERT_INVERTER_GPIO_DEF_PWM_READY_TOP3.port, EVERT_INVERTER_GPIO_DEF_PWM_READY_TOP3.pin);
    io_state.pwm_ready_bot1 = HAL_GPIO_ReadPin(EVERT_INVERTER_GPIO_DEF_PWM_READY_BOT1.port, EVERT_INVERTER_GPIO_DEF_PWM_READY_BOT1.pin);
    io_state.pwm_ready_bot2 = HAL_GPIO_ReadPin(EVERT_INVERTER_GPIO_DEF_PWM_READY_BOT2.port, EVERT_INVERTER_GPIO_DEF_PWM_READY_BOT2.pin);
    io_state.pwm_ready_bot3 = HAL_GPIO_ReadPin(EVERT_INVERTER_GPIO_DEF_PWM_READY_BOT3.port, EVERT_INVERTER_GPIO_DEF_PWM_READY_BOT3.pin);
    io_state.pwm_fault = HAL_GPIO_ReadPin(EVERT_INVERTER_GPIO_DEF_PWM_FAULT.port, EVERT_INVERTER_GPIO_DEF_PWM_FAULT.pin);
    io_state.pwm_fan_fault = HAL_GPIO_ReadPin(EVERT_INVERTER_GPIO_DEF_PWM_FAN_FAULT.port, EVERT_INVERTER_GPIO_DEF_PWM_FAN_FAULT.pin);
    io_state.pwm_fdcan_fault = HAL_GPIO_ReadPin(EVERT_INVERTER_GPIO_DEF_PWM_FDCAN_FAULT.port, EVERT_INVERTER_GPIO_DEF_PWM_FDCAN_FAULT.pin);

    EVERT_INVERTER_ISR_HF_Readings();

    // TODO: Testing
    static float32_t dutyU, dutyV, dutyW;

    // Call in your ISR
    GetDutyCycles(&dutyU, &dutyV, &dutyW);
    // float32_t duty_cycle_u = GetDutyCycle(0);
    // float32_t duty_cycle_v = GetDutyCycle(+120);
    // float32_t duty_cycle_w = GetDutyCycle(+240);
    EVERT_INVERTER_SetDutyCycle(dutyU, dutyV, dutyW);

    // // Summary of Steps for Bi-Directional PFC Implementation:
    // // 1. Ensure DQ frame alignment for both power-sourcing and power-sinking modes.
    // // 2. Modify current control logic to handle bi-directional current flow and adjust the idRef and iqRef values dynamically.
    // // 3. Implement mode detection for switching between inverter and rectifier (PFC) modes.
    // // 4. Control reactive power (PFC) in both directions by adjusting the iq component of the current.
    // // 5. Adjust control loops for smooth operation in both directions (inverter and rectifier).
    // // 6. Implement DC bus voltage regulation for both power-sourcing and power-sinking operations.
    // // 7. Add slew rate and current limiting for smooth transitions between modes.
    // // 8. Update duty cycle calculations for bi-directional current flow.
    // // 9. Maintain grid synchronization for accurate PFC in both directions.

    // // Grid voltage and current transforms
    // EVERT_INVERTER_TransformAbcToDq0(&gf_inverter_current_dq0, uf_current_u, uf_current_v, uf_current_w, gf_sine, gf_cosine);
    // EVERT_INVERTER_TransformAbcToDq0(&gf_grid_voltage_dq0, uf_voltage_grid_u, uf_voltage_grid_v, uf_voltage_grid_w, gf_sine, gf_cosine);

    // // PWM output
    // if (gf_start_pwm_output)
    // {
    //     gf_start_pwm_output = false;
    //     gf_closed_current_loop = true;
    //     EVERT_INVERTER_SetPwmEnabled(true);
    // }

    // // Closed-loop current control
    // if (gf_closed_current_loop)
    // {
    //     EVERT_INVERTER_GridFormingClosedCurrentLoop();
    // }

    // // Calculate the resulting voltage based on the current control
    // EVERT_INVERTER_TransformDq0ToAbc(&gf_inverter_voltage_abc, gf_vd_inverter_pu, gf_vq_inverter_pu, gf_sine, gf_cosine);

    // // TODO: Skip for now
    // // [] TINV_Third_Harmonic_Injection [TINV_duty_THI_pu]
    // // [] TINV_MIDDLE_POINT_CONTROL_STATUS - middle point control [TINV_duty_0_pu]

    // // Update PWM duty cycles
    // if (gf_closed_current_loop)
    // {
    //     // Update PWM duty cycles
    //     gf_duty_cycle_a_pu = gf_inverter_voltage_abc.a; // + spll_duty_cycle_thi + spll_duty_cycle_0;
    //     gf_duty_cycle_b_pu = gf_inverter_voltage_abc.b; // + spll_duty_cycle_thi + spll_duty_cycle_0;
    //     gf_duty_cycle_c_pu = gf_inverter_voltage_abc.c; // + spll_duty_cycle_thi + spll_duty_cycle_0;

    //     // Calculate the duty cycle percentage and pwm signals
    //     float32_t a = ((EVERT_SETTING_INVERTER_VOLTAGE_INSTANTANEOUS_MAX * gf_duty_cycle_a_pu) + EVERT_SETTING_INVERTER_VOLTAGE_INSTANTANEOUS_MAX) / (EVERT_SETTING_INVERTER_VOLTAGE_INSTANTANEOUS_MAX * 2.0f);
    //     float32_t b = ((EVERT_SETTING_INVERTER_VOLTAGE_INSTANTANEOUS_MAX * gf_duty_cycle_b_pu) + EVERT_SETTING_INVERTER_VOLTAGE_INSTANTANEOUS_MAX) / (EVERT_SETTING_INVERTER_VOLTAGE_INSTANTANEOUS_MAX * 2.0f);
    //     float32_t c = ((EVERT_SETTING_INVERTER_VOLTAGE_INSTANTANEOUS_MAX * gf_duty_cycle_c_pu) + EVERT_SETTING_INVERTER_VOLTAGE_INSTANTANEOUS_MAX) / (EVERT_SETTING_INVERTER_VOLTAGE_INSTANTANEOUS_MAX * 2.0f);
    //     float32_t a_1 = EVERT_INVERTER_GetDutyCycleCcrByPercentage(a);
    //     float32_t b_1 = EVERT_INVERTER_GetDutyCycleCcrByPercentage(b);
    //     float32_t c_1 = EVERT_INVERTER_GetDutyCycleCcrByPercentage(c);

    //     // Set Dead Time
    //     // TODO: Dead time adjustments is not implemented yet

    //     // Set PWM duty cycle
    //     EVERT_INVERTER_SetDutyCycle(a_1, b_1, c_1);
    // }
    // else
    // {
    //     // 50% duty cycle
    //     EVERT_INVERTER_SetDutyCycle(0.5f, 0.5f, 0.5f);

    //     // TODO: Dead time adjustments is not implemented yet
    //     EVERT_INVERTER_SetDeadtime(HRTIM_TIMERINDEX_TIMER_A, 100);
    //     EVERT_INVERTER_SetDeadtime(HRTIM_TIMERINDEX_TIMER_B, 100);
    //     EVERT_INVERTER_SetDeadtime(HRTIM_TIMERINDEX_TIMER_B, 100);
    // }

    // // Run SRF-PLL control and Update angle in radians
    // EVERT_INVERTER_GridFormingSpllSrfControl();

    // // Calculate sine and cosine using CORDIC
    // static const float32_t CONSTANT = 2147483648.0f / M_PI;
    // LL_CORDIC_WriteData(CORDIC, (int32_t)(gf_angle_radians * CONSTANT));
    // gf_sine = (float32_t)((int32_t)LL_CORDIC_ReadData(CORDIC) / 2147483648.0f);   /* Read sine */
    // gf_cosine = (float32_t)((int32_t)LL_CORDIC_ReadData(CORDIC) / 2147483648.0f); /* Read cosine */

    // // Check for zero-crossing
    // EVERT_INVERTER_GridFormingSpllCheckSynchronization();

    // // TODO: Constraints checking

    // Set ADC conversion flag
    adc_completed[0] = false;
    adc_completed[1] = false;
}

void EVERT_INVERTER_ISR_100HZ_IRQHandler()
{
    if (adc_completed[2])
    {
        EVERT_INVERTER_ISR_LF_Readings();

        adc_completed[2] = false;
    }
}

void __overrides HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc == &hadc1)
    {
        adc_completed[0] = true;
    }
    else if (hadc == &hadc2)
    {
        adc_completed[1] = true;
    }
    else if (hadc == &hadc3)
    {
        adc_completed[2] = true;
    }
}

// Error Callbacks
void EVERT_INVERTER_hal_error(char *error_message)
{
    EVERT_INVERTER_CommonErrorCallback(error_message);
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    UNUSED(hadc);
    EVERT_INVERTER_CommonErrorCallback("ADC Error\n");
}

void HAL_DMA_ErrorCallback(DMA_HandleTypeDef *hdma)
{
    UNUSED(hdma);
    EVERT_INVERTER_CommonErrorCallback("DMA Error\n");
}

void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan)
{
    UNUSED(hfdcan);
    EVERT_INVERTER_CommonErrorCallback("FDCAN Error\n");
}

void HAL_FLASH_OperationErrorCallback(uint32_t ReturnValue)
{
    UNUSED(ReturnValue);
    EVERT_INVERTER_CommonErrorCallback("FLASH Error\n");
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    UNUSED(hi2c);
    EVERT_INVERTER_CommonErrorCallback("I2C Error\n");
}

void HAL_TIM_ErrorCallback(TIM_HandleTypeDef *htim)
{
    UNUSED(htim);
    EVERT_INVERTER_CommonErrorCallback("TIM Error\n");
}

void EVERT_INVERTER_CommonErrorCallback(char *error_message)
{
    EVERT_HAL_BreakPoint(error_message);
    EVERT_INVERTER_SetStatusLed(0, EVERT_CONSTANT_INVERTER_STATUS_LED0_COLOR_ERROR, EVERT_CONSTANT_INVERTER_STATUS_LED0_TIME_ERROR_ON, EVERT_CONSTANT_INVERTER_STATUS_LED0_TIME_ERROR_OFF);
    EVERT_INVERTER_SetStatusLed(1, EVERT_CONSTANT_INVERTER_STATUS_LED1_COLOR_ERROR, EVERT_CONSTANT_INVERTER_STATUS_LED1_TIME_ERROR_ON, EVERT_CONSTANT_INVERTER_STATUS_LED1_TIME_ERROR_OFF);

    __disable_irq();

    while (1)
    {
    }
}