//
// Description: FZ2812 LED driver configuration.
// Created: 2021.08.17
//
// WS2812 timing requires a PWM frequency of around 800 kHz, which means a period of 1.25 µs.
// "0" bit: ~33% duty cycle (e.g., 0.4 µs high, 0.85 µs low).
// "1" bit: ~66% duty cycle (e.g., 0.8 µs high, 0.45 µs low).

#ifndef EVERT_CORE_FZ2812_H_
#define EVERT_CORE_FZ2812_H_

#include <stm32g4xx_hal.h>
#include "_conf_evert_hal.h"

#define EVERT_FZ2812_LED_COUNT (EVERT_HAL_CONF_FZ2812_COUNT)

#if EVERT_FZ2812_LED_COUNT > 0

#define EVERT_FZ2812_RGB_DATA_LENGTH_BITS_RESET (25) // n-bits of reset (30 µs)
#define EVERT_FZ2812_RGB_DATA_LENGTH_BITS (24)       // 24-bits of data per LED
#define EVERT_FZ2812_RGB_DATA_LENGTH_BITS_TOTAL ((EVERT_FZ2812_RGB_DATA_LENGTH_BITS * EVERT_HAL_CONF_FZ2812_COUNT) + EVERT_FZ2812_RGB_DATA_LENGTH_BITS_RESET)
#define EVERT_FZ2812_RGB_DATA_LENGTH_BYTES ((EVERT_FZ2812_RGB_DATA_LENGTH_BITS * EVERT_HAL_CONF_FZ2812_COUNT) / 8)
#define EVERT_FZ2812_PWM_FREQUENCY (800000f)                                                                                          // 800kHz
#define EVERT_FZ2812_PWM_PERIOD_TICKS (212)                                                                                           //
#define EVERT_FZ2812_PWM_PERIOD (1.25f)                                                                                               // 1.25us
#define EVERT_FZ2812_PWM_PERIOD_0BIT (0.25f)                                                                                          // 0.4us +- 150ns
#define EVERT_FZ2812_PWM_PERIOD_1BIT (0.75f)                                                                                          // 0.85us +- 150ns
#define EVERT_FZ2812_PWM_CCR_0BIT ((uint8_t)(EVERT_FZ2812_PWM_PERIOD_0BIT / EVERT_FZ2812_PWM_PERIOD * EVERT_FZ2812_PWM_PERIOD_TICKS)) // 33% duty cycle
#define EVERT_FZ2812_PWM_CCR_1BIT ((uint8_t)(EVERT_FZ2812_PWM_PERIOD_1BIT / EVERT_FZ2812_PWM_PERIOD * EVERT_FZ2812_PWM_PERIOD_TICKS)) // 66% duty cycle

#define EVERT_FZ2812_RGB_TO_GRB(rgb) (uint32_t)((((rgb) & 0xFF0000) >> 8) | (((rgb) & 0x00FF00) << 8) | (((rgb) & 0x0000FF) << 8))

typedef struct
{
    uint32_t color;
    bool on;
    uint32_t on_time;
    uint32_t off_time;
    uint32_t on_counter;
    uint32_t off_counter;
} EVERT_FZ2812_LedDataTypeDef;

void EVERT_FZ2812_Init(TIM_HandleTypeDef *htim);
void EVERT_FZ2812_SetBlink(uint8_t led_index, uint32_t on_time, uint32_t off_time);
void EVERT_FZ2812_SetColor(uint8_t led_index, uint32_t grb);
void EVERT_FZ2812_SetColorRgb(uint8_t led_index, uint8_t r, uint8_t g, uint8_t b);
void EVERT_FZ2812_Update(void);

#endif // EVERT_FZ2812_LED_COUNT > 0
#endif // EVERT_CORE_FZ2812_H_