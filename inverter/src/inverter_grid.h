#ifndef EVERT_INVERTER_GRID_H_
#define EVERT_INVERTER_GRID_H_

#include <arm_math.h>
#include <stdbool.h>

#include "inverter_readings.h"
#include "inverter_transforms.h"

#define VD_VQ_SCALING_FACTOR (EVERT_SETTING_INVERTER_VOLTAGE_INSTANTANEOUS_MAX / EVERT_SETTING_INVERTER_VOLTAGE_BUS_MAX)
#define VD_DQ_FACTOR2 (EVERT_SETTING_INVERTER_PLL_DECOUPLING_FACTOR * EVERT_SETTING_INVERTER_GRID_NOMINAL_FREQUENCY)

typedef struct __attribute__((aligned(4)))
{
    // Proportional Gain
    // Determines the controller's response to the error term (difference between reference and feedback).
    // Higher Kp leads to faster response but might cause instability.
    float32_t Kp;

    // Integral Gain
    // Accumulates the error over time and helps eliminate steady-state error.
    // Higher Ki provides better steady-state accuracy but can introduce sluggishness.
    float32_t Ki;

    // Integral Storage
    // Holds the accumulated error term from previous calculations. It's updated within the PI control function.
    float32_t Integral;

    // Saturation Limits
    // Prevents the controller from outputting values that are too high or too low.
    float32_t SaturationHigh;
    float32_t SaturationLow;

    // Saturation Storage
    // Flag (floating-point) that might be set to 1.0f when saturation occurs (control effort reaches the limit) and 0.0f otherwise.
    // This helps prevent the integrator from accumulating error excessively when saturation is active.
    float32_t Saturation;
} EVERT_INVERTER_SpllPiStateTypeDef;

// Transforms
extern EVERT_INVERTER_AbcDq0TypeDef gf_inverter_current_dq0; // Current
extern EVERT_INVERTER_AbcDq0TypeDef gf_grid_voltage_dq0;     // Grid Voltage Reference
extern EVERT_INVERTER_Dq0AbcTypeDef gf_inverter_voltage_abc; // Inverter Voltage

// PI States
extern EVERT_INVERTER_SpllPiStateTypeDef gf_pi_state_d;
extern EVERT_INVERTER_SpllPiStateTypeDef gf_pi_state_q;

// Control flags
extern bool gf_start_pwm_output;    //
extern bool gf_closed_current_loop; // Closed current loop flag

// Sync-zero-crossing
extern float32_t gf_voltage_grid_a_previous;

// Current control
extern float32_t gf_id_ref_pu;      // D-axis reference current
extern float32_t gf_iq_ref_pu;      // Q-axis reference current
extern float32_t gf_id_out;         // D-axis output current from PI controller
extern float32_t gf_iq_out;         // Q-axis output current from PI controller
extern float32_t gf_vd_inverter_pu; // D-axis output voltage from PI controller
extern float32_t gf_vq_inverter_pu; // Q-axis output voltage from PI controller

// Duty cycles
extern float32_t gf_duty_cycle_a_pu;
extern float32_t gf_duty_cycle_b_pu;
extern float32_t gf_duty_cycle_c_pu;

// sine/cosine values
extern float32_t gf_angle_radians; // Angle in radians
extern float32_t gf_sine;          // Sine value
extern float32_t gf_cosine;        // Cosine value

// Synchrounous Reference Frame Data
extern float32_t gf_srf_voltage_q[2];          // Rotating reference frame voltage Q-axis
extern float32_t gf_srf_loop_filter_output[2]; // Loop filter output
extern float32_t gf_srf_output_frequency;      // Output frequency
extern float32_t gf_srf_nominal_frequency;     // Nominal grid frequency
extern float32_t gf_srf_theta[2];              // Grid phase angle
extern float32_t gf_srf_delta_t;               // ISR time step
extern float32_t gf_srf_filter_coeff_b0;       // Filter coefficient
extern float32_t gf_srf_filter_coeff_b1;       // Filter coefficient

void EVERT_INVERTER_GridFormingInit(const float32_t kp, const float32_t ki, const float32_t coeff_b0, const float32_t coeff_b1);

static inline void EVERT_INVERTER_GridFormingPiStateInit(EVERT_INVERTER_SpllPiStateTypeDef *pi_state, const float32_t kp, const float32_t ki)
{
    pi_state->Kp = kp;
    pi_state->Ki = ki;
    pi_state->Integral = 0;
    pi_state->SaturationHigh = 0;
    pi_state->SaturationLow = 0;
    pi_state->Saturation = 0;
}

static inline float32_t EVERT_INVERTER_GridFormingPiControl(EVERT_INVERTER_SpllPiStateTypeDef *pi_state, float32_t setpoint_reference, float32_t feedback_value)
{
    // TIDA1606 reference
    // float32_t v2, v4, v5, v9;
    // v2 = p->Kp * (rk - yk);
    // v4 = p->i10 + (p->Ki * p->i6 * v2);
    // v5 = v2 + v4;
    // v9 = (v5 > p->Umax) ? p->Umax : v5;
    // v9 = (v9 < p->Umin) ? p->Umin : v9;
    // p->i10 = v4;
    // p->i6 = (v5 == v9) ? 1.0f : 0.0f;

    float32_t error = setpoint_reference - feedback_value;
    float32_t proportional_term = pi_state->Kp * error;
    float32_t integral_term = pi_state->Integral + (pi_state->Ki * pi_state->Saturation * proportional_term);
    float32_t control_effort = proportional_term + integral_term;
    float32_t control_effort_final = (control_effort > pi_state->SaturationHigh) ? pi_state->SaturationHigh : control_effort;
    EVERT_INVERTER_MATH_CLAMP(control_effort_final, pi_state->SaturationLow, pi_state->SaturationHigh);

    pi_state->Integral = integral_term;
    pi_state->Saturation = (control_effort == control_effort_final) ? 1.0f : 0.0f;

    return control_effort_final;
}

static inline void EVERT_INVERTER_GridFormingClosedCurrentLoop()
{
    // Run PI controller on the d and q axis currents
    gf_id_out = EVERT_INVERTER_GridFormingPiControl(&gf_pi_state_d, gf_id_ref_pu, gf_inverter_current_dq0.d);
    gf_iq_out = EVERT_INVERTER_GridFormingPiControl(&gf_pi_state_q, gf_iq_ref_pu, gf_inverter_current_dq0.q);

    // TIDA1606 reference
    // * TINV_gi_id_out: The output of the d axis current controller.
    // * TINV_vGrid_dq0_pos.d: The d axis component of the grid voltage in the d-q reference frame.
    // * (TINV_VGRID_MAX_SENSE_VOLTS / TINV_VBUS_MAX_SENSE_VOLTS): A scaling factor between the maximum sensed
    //   grid voltage and the maximum sensed DC bus voltage. This converts grid voltage into the same scale as the DC bus voltage.
    // * TINV_iInv_dq0_pos.q * TINV_DECOUPLING_CONST * TINV_AC_FREQ_HZ: Represents a decoupling term for the control loop.
    //   In rotating reference frames, the q axis current (iq) affects the d axis voltage (vd). This term compensates for this cross-coupling effect.
    // * TINV_vBus_sensed_FilteredAndClamped_pu * 0.5f: This term normalizes the voltage by dividing by the sensed DC bus voltage (in per-unit)
    //   multiplied by 0.5. This scaling ensures the voltage commands remain within the desired range relative to the DC bus voltage.

    // float voltage_bus = evert_gf_voltage_bus;

    // TODO: Optimization
    // 5200 without this alone
    gf_vd_inverter_pu = (gf_id_out + (gf_grid_voltage_dq0.d * VD_VQ_SCALING_FACTOR) - (gf_inverter_current_dq0.q * VD_DQ_FACTOR2)) / (uf_bus_voltage * 0.5f);

    // 5200 ish without this alone
    gf_vq_inverter_pu = (gf_iq_out + (gf_grid_voltage_dq0.q * VD_VQ_SCALING_FACTOR) + (gf_inverter_current_dq0.d * VD_DQ_FACTOR2)) / (uf_bus_voltage * 0.5f);

    // 3100 without any of them and no volatile
    // 3500-3700 without any of them
    // 5500 with both and no volatile
    // 6600+ with both

    EVERT_INVERTER_MATH_CLAMP(gf_vd_inverter_pu, -1.0f, 1.0f);
    EVERT_INVERTER_MATH_CLAMP(gf_vq_inverter_pu, -1.0f, 1.0f);
}

static inline void EVERT_INVERTER_GridFormingSpllSrfControl()
{
    // Update the srf_state->voltage_q[0] with the grid value
    gf_srf_voltage_q[0] = gf_grid_voltage_dq0.q;

    // Loop Filter
    gf_srf_loop_filter_output[0] = gf_srf_loop_filter_output[1] + (gf_srf_filter_coeff_b0 * gf_srf_voltage_q[0]) + (gf_srf_filter_coeff_b1 * gf_srf_voltage_q[1]);

    // Update previous states for the next iteration
    gf_srf_loop_filter_output[1] = gf_srf_loop_filter_output[0];
    gf_srf_voltage_q[1] = gf_srf_voltage_q[0];

    // Output clamping to prevent overflows
    // TODO: 200.0f is a magic number, should be replaced with a constant - and described
    // 
    static const float32_t MAX_LOOP_FILTER_OUTPUT = 200.0f;
    gf_srf_loop_filter_output[0] = (gf_srf_loop_filter_output[0] > MAX_LOOP_FILTER_OUTPUT) ? MAX_LOOP_FILTER_OUTPUT : gf_srf_loop_filter_output[0];

    // VCO - Voltage Controlled Oscillator
    gf_srf_output_frequency = gf_srf_nominal_frequency + gf_srf_loop_filter_output[0];

    // Phase accumulator for the PLL
    gf_srf_theta[0] = gf_srf_theta[1] + (gf_srf_output_frequency * gf_srf_delta_t * 2.0f * 3.1415926f);

    // Wrap phase theta[0] to 0-2pi
    if (gf_srf_theta[0] > 2.0f * 3.1415926f)
    {
        gf_srf_theta[0] -= 2.0f * 3.1415926f;
    }

    // Update previous state for theta
    gf_srf_theta[1] = gf_srf_theta[0];
    gf_angle_radians = gf_srf_theta[1];
}

static inline void EVERT_INVERTER_GridFormingSpllCheckSynchronization()
{
    // Why Perform Zero-Crossing Detection?
    // Zero-crossing detection is a crucial aspect of grid-tied inverter operation for several reasons:
    // Safe Synchronization:
    // 1. Smooth Connection: Synchronizing the inverter at a zero-crossing ensures that the connection between the inverter and the grid is smooth. At zero-crossing, the voltage is momentarily zero, meaning there is minimal instantaneous voltage difference between the inverter and grid, reducing inrush currents and stress on components.
    // Phase Alignment: It helps ensure that the inverter’s output voltage phase aligns correctly with the grid voltage phase. Proper phase alignment is essential for efficient power transfer and to avoid damaging the inverter or grid infrastructure.
    // Avoiding Transients and Surges:
    // 2. Connecting the inverter to the grid when there is a significant voltage difference (like at the peak of the voltage waveform) can cause large transients or surges. By waiting for a zero-crossing, the inverter can start its operation with minimal disturbance to the grid, avoiding potential surges that could trip protections or damage equipment.
    // Controlled Start-Up:
    // 3. Timing Control: The zero-crossing provides a predictable point for starting the inverter, which is important for control algorithms that need to synchronize precisely with the grid’s AC waveform.
    // Feedback Initialization: Starting at zero-crossing helps initialize the feedback control loops in a known state, avoiding potential control instability that might occur if starting at an arbitrary point on the waveform.

    // Check for zero-crossing (grid voltage near zero)
    if (uf_voltage_grid_u > 0.0f && gf_voltage_grid_a_previous < 0.0f)
    {
        // Zero-crossing detected
        gf_closed_current_loop = true;
        gf_start_pwm_output = true;
    }
}

#endif // EVERT_INVERTER_GRID_H_