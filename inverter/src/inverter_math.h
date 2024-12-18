#ifndef EVERT_INVERTER_MATH_H_
#define EVERT_INVERTER_MATH_H_

// Inverter Constants
#define EVERT_INVERTER_MATH_K_INDUCTOR_VALUE (0.000340f) // 340uH

// Math Constants
#define EVERT_INVERTER_MATH_K_2_OVER_3 (2.0f / 3.0f)
#define EVERT_INVERTER_MATH_K_1_OVER_3 (1.0f / 3.0f)
#define EVERT_INVERTER_MATH_K_1_OVER_SQRT3 (1.0f / 1.7320508075688772f)

// Math Functions
#define EVERT_INVERTER_MATH_CLAMP(value, min, max) ((value) = (((value) > (max)) ? (max) : (((value) < (min)) ? (min) : (value))))
#define EVERT_INVERTER_MATH_NORMALIZED_CLAMP(value) (value > 1.0f ? 1.0f : (value < -1.0f ? -1.0f : value))
#define EVERT_INVERTER_MATH_EMA(new_value, prev_value, multiplier) (prev_value = ((new_value * (1.0f - multiplier)) + (prev_value * multiplier)))

#endif // EVERT_INVERTER_MATH_H_