#include "ws2812b.h"

#include "main.h"
#include "tim.h"

#include <stdint.h>
#include <string.h>

#define WS2812_LED_COUNT       12U

/*
 * TIM1 clock = 72 MHz
 * ARR = 89
 *
 * Jeden bit:
 * 72 MHz / 90 = 800 kHz
 * okres bitu = 1.25 us
 *
 * Dla WS2812B:
 * 0 -> high ok. 0.4 us
 * 1 -> high ok. 0.8 us
 *
 * Trochę bezpieczniejsze wartości niż 29/58:
 * 28/90 = 0.389 us
 * 56/90 = 0.778 us
 */
#define WS2812_PWM_ZERO        28U
#define WS2812_PWM_ONE         56U
#define WS2812_RESET_SLOTS     100U

#define WS2812_BITS_PER_LED    24U
#define WS2812_BUFFER_SIZE     ((WS2812_LED_COUNT * WS2812_BITS_PER_LED) + WS2812_RESET_SLOTS)

static uint16_t ws2812_pwm_buffer[WS2812_BUFFER_SIZE];

static volatile uint8_t ws2812_busy = 0;

static uint8_t applyBrightness(uint8_t color, uint8_t brightness)
{
    return static_cast<uint8_t>((static_cast<uint16_t>(color) * brightness) / 255U);
}

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

static void fillLedColor(uint8_t red,
                         uint8_t green,
                         uint8_t blue,
                         uint16_t *buffer,
                         uint16_t *index)
{
    /*
     * U Ciebie pierwsza dioda dla L,255,255,0,0; świeci na czerwono,
     * więc kolejność GRB wygląda poprawnie.
     *
     * WS2812B: G7..G0, R7..R0, B7..B0
     */
    putByteToBuffer(green, buffer, index);
    putByteToBuffer(red, buffer, index);
    putByteToBuffer(blue, buffer, index);
}

void WS2812_Init(void)
{
    memset(ws2812_pwm_buffer, 0, sizeof(ws2812_pwm_buffer));

    ws2812_busy = 0;

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);

    /*
     * Na starcie gaś wszystkie LED-y.
     */
    WS2812_SetActiveLeds(0, 0, 0, 0, 0);
}

uint8_t WS2812_IsBusy(void)
{
    return ws2812_busy;
}

void WS2812_SetActiveLeds(uint8_t activeLedCount,
                          uint8_t brightness,
                          uint8_t red,
                          uint8_t green,
                          uint8_t blue)
{
    if (activeLedCount > WS2812_LED_COUNT)
    {
        activeLedCount = WS2812_LED_COUNT;
    }

    /*
     * Dla 12 LED transmisja trwa ok.:
     * 12 * 24 * 1.25 us = 360 us + reset.
     *
     * Jeśli poprzednia transmisja trwa, pomijamy nową.
     */
    if (ws2812_busy)
    {
        return;
    }

    uint8_t scaledRed = applyBrightness(red, brightness);
    uint8_t scaledGreen = applyBrightness(green, brightness);
    uint8_t scaledBlue = applyBrightness(blue, brightness);

    uint16_t index = 0;

    for (uint8_t led = 0; led < WS2812_LED_COUNT; led++)
    {
        if (led < activeLedCount)
        {
            fillLedColor(scaledRed,
                         scaledGreen,
                         scaledBlue,
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

    /*
     * Reset/latch: stan niski przez >50 us.
     * 100 slotów * 1.25 us = 125 us.
     */
    for (uint16_t i = 0; i < WS2812_RESET_SLOTS; i++)
    {
        ws2812_pwm_buffer[index++] = 0;
    }

    ws2812_busy = 1;

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

        /*
         * Bardzo ważne:
         * po transmisji wymuszamy stan niski na PWM.
         */
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);

        ws2812_busy = 0;
    }
}
