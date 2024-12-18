#include "inverter_grid.h"
#include "inverter_readings.h"

// Transforms
EVERT_INVERTER_AbcDq0TypeDef gf_inverter_current_dq0; // Current
EVERT_INVERTER_AbcDq0TypeDef gf_grid_voltage_dq0;     // Grid Voltage Reference
EVERT_INVERTER_Dq0AbcTypeDef gf_inverter_voltage_abc; // Inverter Voltage

EVERT_INVERTER_SpllPiStateTypeDef gf_pi_state_d;
EVERT_INVERTER_SpllPiStateTypeDef gf_pi_state_q;

// Control flags
bool gf_start_pwm_output;    //
bool gf_closed_current_loop; // Closed current loop flag

// Sync-zero-crossing
float32_t gf_voltage_grid_a_previous;

// Current control
float32_t gf_id_ref_pu;      // D-axis reference current
float32_t gf_iq_ref_pu;      // Q-axis reference current
float32_t gf_id_out;         // D-axis output current from PI controller
float32_t gf_iq_out;         // Q-axis output current from PI controller
float32_t gf_vd_inverter_pu; // D-axis output voltage from PI controller
float32_t gf_vq_inverter_pu; // Q-axis output voltage from PI controller

// Duty cycles
float32_t gf_duty_cycle_a_pu;
float32_t gf_duty_cycle_b_pu;
float32_t gf_duty_cycle_c_pu;

// sine/cosine values
float32_t gf_angle_radians; // Angle in radians
float32_t gf_sine;          // Sine value
float32_t gf_cosine;        // Cosine value

// Synchrounous Reference Frame Data
float32_t gf_srf_voltage_q[2];          // Rotating reference frame voltage Q-axis
float32_t gf_srf_loop_filter_output[2]; // Loop filter output
float32_t gf_srf_output_frequency;      // Output frequency
float32_t gf_srf_nominal_frequency;     // Nominal grid frequency
float32_t gf_srf_theta[2];              // Grid phase angle
float32_t gf_srf_delta_t;               // ISR time step
float32_t gf_srf_filter_coeff_b0;       // Filter coefficient
float32_t gf_srf_filter_coeff_b1;       // Filter coefficient

void EVERT_INVERTER_GridFormingInit(const float32_t ki, const float32_t kp, const float32_t coeff_b0, const float32_t coeff_b1)
{
    // Initialize the grid forming transforms
    EVERT_INVERTER_InitAbcDq0(&gf_inverter_current_dq0);
    EVERT_INVERTER_InitAbcDq0(&gf_grid_voltage_dq0);
    EVERT_INVERTER_InitDq0Abc(&gf_inverter_voltage_abc);

    // Initialize PI States
    EVERT_INVERTER_GridFormingPiStateInit(&gf_pi_state_d, ki, kp);
    EVERT_INVERTER_GridFormingPiStateInit(&gf_pi_state_q, ki, kp);

    // Control flags
    gf_start_pwm_output = false;
    gf_closed_current_loop = false;

    // Sync-zero-crossing
    gf_voltage_grid_a_previous = 0;

    // Current control
    gf_id_ref_pu = 0.005f; // D-axis reference current
    gf_iq_ref_pu = 0;      // Q-axis reference current
    gf_id_out = 0;         // D-axis output current from PI controller
    gf_iq_out = 0;         // Q-axis output current from PI controller
    gf_vd_inverter_pu = 0; // D-axis output voltage from PI controller
    gf_vq_inverter_pu = 0; // Q-axis output voltage from PI controller

    // Duty cycles
    gf_duty_cycle_a_pu = 0;
    gf_duty_cycle_b_pu = 0;
    gf_duty_cycle_c_pu = 0;

    // sine/cosine values
    gf_angle_radians = 0; // Angle in radians
    gf_sine = 0;          // Sine value
    gf_cosine = 0;        // Cosine value

    // Synchrounous Reference Frame Data
    gf_srf_voltage_q[0] = 0;           // Rotating reference frame voltage Q-axis
    gf_srf_voltage_q[1] = 0;           // Rotating reference frame voltage Q-axis
    gf_srf_loop_filter_output[0] = 0;  // Loop filter output
    gf_srf_loop_filter_output[1] = 0;  // Loop filter output
    gf_srf_output_frequency = 0;       // Output frequency
    gf_srf_nominal_frequency = 0;      // Nominal grid frequency
    gf_srf_theta[0] = 0;               // Grid phase angle
    gf_srf_theta[1] = 0;               // Grid phase angle
    gf_srf_delta_t = 0;                // ISR time step
    gf_srf_filter_coeff_b0 = coeff_b0; // Filter coefficient
    gf_srf_filter_coeff_b1 = coeff_b1; // Filter coefficient
}
