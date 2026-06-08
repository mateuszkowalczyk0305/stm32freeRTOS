#include "ws2812b.h"

#include "main.h"
#include "tim.h"

#include <stdint.h>
#include <string.h>

#define WS2812_LED_COUNT       12U

/*
 * TIM1 clock = 72 MHz
 * ARR = 89, czyli 90 taktow na bit.
 *
 * 72 MHz / 90 = 800 kHz
 * okres bitu = 1.25 us
 *
 * Przeliczenie wg noty WS2812B:
 * T0H typ 0.40 us -> 0.40 us / 13.888 ns = 28.8 -> 29
 * T1H typ 0.80 us -> 0.80 us / 13.888 ns = 57.6 -> 58
 */
#define WS2812_PWM_ZERO        29U
#define WS2812_PWM_ONE         58U
#define WS2812_RESET_SLOTS     280U

#define WS2812_BITS_PER_LED    24U
#define WS2812_DATA_SLOTS      (WS2812_LED_COUNT * WS2812_BITS_PER_LED)
#define WS2812_BUFFER_SIZE     (WS2812_RESET_SLOTS + WS2812_DATA_SLOTS + WS2812_RESET_SLOTS)

static uint16_t ws2812_pwm_buffer[WS2812_BUFFER_SIZE];

static volatile uint8_t ws2812_busy = 0;

static void putByteToBuffer(uint8_t byte, uint16_t *buffer, uint16_t *index)
{
    for (int8_t bit = 7; bit >= 0; bit--)
    {
        if ((byte & (1U << bit)) != 0U)
        {
            buffer[*index] = WS2812_PWM_ONE;
        }
        else
        {
            buffer[*index] = WS2812_PWM_ZERO;
        }

        (*index)++;
    }
}

static void fillResetSlots(uint16_t *buffer, uint16_t *index)
{
    for (uint16_t resetSlot = 0; resetSlot < WS2812_RESET_SLOTS; resetSlot++)
    {
        buffer[*index] = 0;
        (*index)++;
    }
}

static void fillLedColor(uint8_t red,
                         uint8_t green,
                         uint8_t blue,
                         uint16_t *buffer,
                         uint16_t *index)
{
    /* WS2812B: G7..G0, R7..R0, B7..B0 */
    putByteToBuffer(green, buffer, index);
    putByteToBuffer(red, buffer, index);
    putByteToBuffer(blue, buffer, index);
}

void WS2812_Init(void)
{
    memset(ws2812_pwm_buffer, 0, sizeof(ws2812_pwm_buffer));

    ws2812_busy = 0;

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COUNTER(&htim1, 0);

    WS2812_SetActiveLeds(0, 0, 0, 0);
}

uint8_t WS2812_IsBusy(void)
{
    return ws2812_busy;
}

void WS2812_SetActiveLeds(uint8_t activeLedCount,
                          uint8_t red,
                          uint8_t green,
                          uint8_t blue)
{
    if (activeLedCount > WS2812_LED_COUNT)
    {
        activeLedCount = WS2812_LED_COUNT;
    }

    if (ws2812_busy)
    {
        return;
    }

    uint16_t index = 0;

    fillResetSlots(ws2812_pwm_buffer, &index);

    for (uint8_t led = 0; led < WS2812_LED_COUNT; led++)
    {
        if (led < activeLedCount)
        {
            fillLedColor(red,
                         green,
                         blue,
                         ws2812_pwm_buffer,
                         &index);
        }
        else
        {
            fillLedColor(0,
                         0,
                         0,
                         ws2812_pwm_buffer,
                         &index);
        }
    }

    fillResetSlots(ws2812_pwm_buffer, &index);

    ws2812_busy = 1;

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COUNTER(&htim1, 0);

    HAL_StatusTypeDef status = HAL_TIM_PWM_Start_DMA(&htim1,
                                                     TIM_CHANNEL_2,
                                                     (uint32_t*)ws2812_pwm_buffer,
                                                     WS2812_BUFFER_SIZE);

    if (status != HAL_OK)
    {
        ws2812_busy = 0;
    }
}

extern "C" void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1)
    {
        HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_2);

        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
        __HAL_TIM_SET_COUNTER(&htim1, 0);

        ws2812_busy = 0;
    }
}
