#include "photo_task.h"

#include "cmsis_os.h"
#include "commands.h"

#include <stdint.h>

extern volatile uint16_t adc_dma_buffer[16];
extern volatile uint16_t photo_adc_raw;
extern volatile uint8_t photo_led_count;
extern volatile uint16_t photo_adc_dark_ref;
extern volatile uint16_t photo_adc_light_ref;
extern volatile uint8_t photo_dark_ref_set;
extern volatile uint8_t photo_light_ref_set;

extern osMessageQueueId_t QueueLedCommandsHandle;

#define ADC_BUFFER_SIZE     16U
#define ADC_MAX_VALUE       4095U
#define WS2812_LED_COUNT    12U
#define ADC_IIR_ALPHA_SHIFT 3U
#define ADC_IIR_SCALE_SHIFT 8U

static_assert(sizeof(CommandMessage) == 12, "CommandMessage size must be 12 bytes");

static uint16_t getAdcAverage()
{
    uint32_t sum = 0;

    for (uint8_t i = 0; i < ADC_BUFFER_SIZE; i++)
    {
        sum += adc_dma_buffer[i];
    }

    return static_cast<uint16_t>(sum / ADC_BUFFER_SIZE);
}

static uint16_t filterAdcIir(uint16_t adcValue)
{
    static uint8_t filterInitialized = 0;
    static uint32_t filteredValue = 0;

    uint32_t inputValue = static_cast<uint32_t>(adcValue) << ADC_IIR_SCALE_SHIFT;

    if (filterInitialized == 0U)
    {
        filteredValue = inputValue;
        filterInitialized = 1U;
    }
    else if (inputValue > filteredValue)
    {
        filteredValue += (inputValue - filteredValue) >> ADC_IIR_ALPHA_SHIFT;
    }
    else
    {
        filteredValue -= (filteredValue - inputValue) >> ADC_IIR_ALPHA_SHIFT;
    }

    return static_cast<uint16_t>((filteredValue + (1U << (ADC_IIR_SCALE_SHIFT - 1U))) >> ADC_IIR_SCALE_SHIFT);
}

static uint8_t mapAdcToLedCount(uint16_t adcValue)
{
    if ((photo_dark_ref_set == 0U) || (photo_light_ref_set == 0U))
    {
        return 0U;
    }

    uint16_t darkRef = photo_adc_dark_ref;
    uint16_t lightRef = photo_adc_light_ref;

    if (darkRef > ADC_MAX_VALUE)
    {
        darkRef = ADC_MAX_VALUE;
    }

    if (lightRef > ADC_MAX_VALUE)
    {
        lightRef = ADC_MAX_VALUE;
    }

    if (lightRef <= darkRef)
    {
        if (darkRef < ADC_MAX_VALUE)
        {
            lightRef = darkRef + 1U;
        }
        else
        {
            darkRef = ADC_MAX_VALUE - 1U;
            lightRef = ADC_MAX_VALUE;
        }
    }

    if (adcValue <= darkRef)
    {
        return 0U;
    }

    if (adcValue >= lightRef)
    {
        return WS2812_LED_COUNT;
    }

    uint32_t span = static_cast<uint32_t>(lightRef - darkRef);
    uint32_t relative = static_cast<uint32_t>(adcValue - darkRef);
    uint8_t mapped = static_cast<uint8_t>((relative * WS2812_LED_COUNT) / span);

    if (mapped == 0U)
    {
        mapped = 1U;
    }

    return mapped;
}

extern "C" void Photo_Task(void *argument)
{
    uint8_t lastLedCount = 255;

    while (1)
    {
        uint16_t adcAverage = getAdcAverage();
        uint16_t adcFiltered = filterAdcIir(adcAverage);
        uint8_t ledCount = mapAdcToLedCount(adcFiltered);

        photo_adc_raw = adcFiltered;
        photo_led_count = ledCount;

        if (ledCount != lastLedCount)
        {
            CommandMessage msg = {};

            msg.id = CMD_ID_SET_LED_COUNT;
            msg.adcRaw = adcFiltered;
            msg.ledCount = ledCount;

            osMessageQueuePut(QueueLedCommandsHandle, &msg, 0, 0);

            lastLedCount = ledCount;
        }

        osDelay(100);
    }
}
