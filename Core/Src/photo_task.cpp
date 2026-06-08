#include "photo_task.h"

#include "cmsis_os.h"
#include "commands.h"

#include <stdint.h>

extern volatile uint16_t adc_dma_buffer[16];
extern volatile uint16_t photo_adc_raw;
extern volatile uint8_t photo_led_count;

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
    if (adcValue > ADC_MAX_VALUE)
    {
        adcValue = ADC_MAX_VALUE;
    }

    return static_cast<uint8_t>((adcValue * WS2812_LED_COUNT) / ADC_MAX_VALUE);
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
