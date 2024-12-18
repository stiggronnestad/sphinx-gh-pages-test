#include <stdbool.h>
#include "fz2812.h"

#if EVERT_FZ2812_LED_COUNT > 0

#define EVERT_FZ2812_RGB_BUFFER_LENGTH (EVERT_FZ2812_RGB_DATA_LENGTH_BITS_TOTAL)
#define TIM2_CCR1_ADDRESS ((uint32_t) & TIM2->CCR1)

static TIM_HandleTypeDef *fz2812_htim;
static uint32_t fz2812_buffer[EVERT_FZ2812_RGB_BUFFER_LENGTH] = {0};
static EVERT_FZ2812_LedDataTypeDef led_data[EVERT_FZ2812_LED_COUNT] = {0};
static bool dma_ready = false;

void EVERT_FZ2812_Init(TIM_HandleTypeDef *htim)
{
    fz2812_htim = htim;

    // Initialize led data
    for (size_t i = 0; i < EVERT_FZ2812_LED_COUNT; i++)
    {
        led_data[i].color = 0;
        led_data[i].on = false;
        led_data[i].on_time = 0;
        led_data[i].off_time = 0;
        led_data[i].on_counter = 0;
        led_data[i].off_counter = 0;
    }
}

void EVERT_FZ2812_SetBlink(uint8_t led_index, uint32_t on_time, uint32_t off_time)
{
    if (led_index >= EVERT_FZ2812_LED_COUNT)
    {
        return;
    }

    led_data[led_index].on_time = on_time;
    led_data[led_index].off_time = off_time;
}

void EVERT_FZ2812_SetColor(uint8_t led_index, uint32_t grb)
{
    if (led_index >= EVERT_FZ2812_LED_COUNT)
    {
        return;
    }

    led_data[led_index].color = grb;
}

void EVERT_FZ2812_SetColorRgb(uint8_t led_index, uint8_t r, uint8_t g, uint8_t b)
{
    if (led_index >= EVERT_FZ2812_LED_COUNT)
    {
        return;
    }

    led_data[led_index].color = (g << 16) | (r << 8) | b;
}

void EVERT_FZ2812_Update(void)
{
    if (!dma_ready)
    {
        return;
    }

    uint32_t buffer_index = 0;

    for (size_t i = 0; i < EVERT_FZ2812_LED_COUNT; i++)
    {
        uint32_t color = led_data[i].color;

        // If the led should be blinking
        if (led_data[i].on_time > 0 && led_data[i].off_time > 0)
        {
            if (led_data[i].on)
            {
                if (led_data[i].on_counter < led_data[i].on_time)
                {
                    led_data[i].on_counter++;
                }
                else
                {
                    led_data[i].on = false;
                    led_data[i].on_counter = 0;
                }
            }
            else
            {
                color = 0;

                if (led_data[i].off_counter < led_data[i].off_time)
                {
                    led_data[i].off_counter++;
                }
                else
                {
                    led_data[i].on = true;
                    led_data[i].off_counter = 0;
                }
            }
        }

        // Set the color data to the buffer
        for (int j = 23; j >= 0; j--)
        {
            if (color & (1 << j))
            {
                fz2812_buffer[buffer_index] = EVERT_FZ2812_PWM_CCR_1BIT;
            }
            else
            {
                fz2812_buffer[buffer_index] = EVERT_FZ2812_PWM_CCR_0BIT;
            }

            buffer_index++;
        }
    }

    HAL_TIM_PWM_Start_DMA(fz2812_htim, TIM_CHANNEL_1, fz2812_buffer, EVERT_FZ2812_RGB_DATA_LENGTH_BITS_TOTAL);
    dma_ready = false;
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == fz2812_htim)
    {
        HAL_TIM_PWM_Stop_DMA(htim, TIM_CHANNEL_1);
        dma_ready = true;
    }
}

#endif