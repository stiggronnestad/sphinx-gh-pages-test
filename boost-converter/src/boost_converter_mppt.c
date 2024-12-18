#include "boost_converter_mppt.h"
#include "boost_converter_readings.h"

static void EVERT_BOOST_CONVERTER_MpptObserve();
static void EVERT_BOOST_CONVERTER_MpptPerturb();

void EVERT_BOOST_CONVERTER_MpptInit(void)
{
    EVERT_BOOST_CONVERTER_SetDutyCycle(0.0f);

    mppt_state.status = BCS_STANDBY;
    mppt_state.perturb_step = 0.01f;
    mppt_state.current_perturb_step = mppt_state.perturb_step;
    mppt_state.observe_interval = 1000;
    mppt_state.observe_timer = 0.0f;
    mppt_state.previous_duty_cycle = 0.0f;
    mppt_state.previous_power = 0.0f;
}

void EVERT_BOOST_CONVERTER_MpptRun(void)
{
    // 100 Hz ISR = 0.1s = 100 ms
    const float32_t delta_time_ms = 100.0f;
    mppt_state.observe_timer += delta_time_ms;

    if (mppt_state.observe_timer >= mppt_state.observe_interval)
    {
        if (mppt_state.status == BCS_THROTTLE_DOWN)
        {
            EVERT_BOOST_CONVERTER_SetDutyCycle(mppt_state.duty_cycle - mppt_state.perturb_step);
        }
        else if (mppt_state.status == BCS_STANDBY)
        {
            EVERT_BOOST_CONVERTER_SetDutyCycle(0.0f);
        }
        else
        {
            EVERT_BOOST_CONVERTER_MpptObserve();
            EVERT_BOOST_CONVERTER_MpptPerturb();
        }

        mppt_state.observe_timer = 0;
    }
}

void EVERT_BOOST_CONVERTER_MpptObserve()
{
    float32_t current_duty_cycle = mppt_state.duty_cycle;

    // Are we currently cycling up or down?
    // At the start of the MPPT algorithm, we are cycling up from 0.0f starting at 0.1f or something
    bool is_cycling_up = current_duty_cycle > mppt_state.previous_duty_cycle;

    // Absolute the current perturbation size to avoid double negatives (-- = +)
    float32_t perturb_size = mppt_state.current_perturb_step >= 0 ? mppt_state.current_perturb_step : -mppt_state.current_perturb_step;

    // Is the power increasing or decreasing?
    bool power_increasing = fi_power_in > mppt_state.previous_power;

    if (power_increasing)
    {
        // Power is increasing, if we are cycling up, we are on the right track, if not, we need to change direction
        mppt_state.current_perturb_step = is_cycling_up ? perturb_size : -perturb_size;
    }
    else
    {
        // Power is decreasing, if we are cycling up; reverse, if not continue
        mppt_state.current_perturb_step = is_cycling_up ? -perturb_size : +perturb_size;
    }
}

void EVERT_BOOST_CONVERTER_MpptPerturb()
{
    float32_t current_power = fi_power_in;
    float32_t current_duty_cycle = mppt_state.duty_cycle;
    float32_t new_duty_cycle = current_duty_cycle + mppt_state.current_perturb_step;

    // Clamp the new duty cycle
    EVERT_BOOST_CONVERTER_CLAMP(new_duty_cycle, EVERT_CONSTRAINT_BC_DUTY_CYCLE_MPPT_MIN, EVERT_CONSTRAINT_BC_DUTY_CYCLE_MPPT_MAX);

    // Set the new duty cycle
    EVERT_BOOST_CONVERTER_SetDutyCycle(new_duty_cycle);

    // Store the current state
    mppt_state.previous_duty_cycle = current_duty_cycle;
    mppt_state.previous_power = current_power;
}