#include "led_task.h"

#include "cmsis_os.h"
#include "main.h"
#include "gpio.h"
#include "commands.h"
#include "ws2812b.h"

#include <stdbool.h>
#include <stdint.h>

extern osMessageQueueId_t QueueLedCommandsHandle;

static_assert(sizeof(CommandMessage) == 12, "CommandMessage size must be 12 bytes");

typedef struct
{
    uint8_t ledCount;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} LedDebugData;

volatile LedDebugData ledDebug = {};

extern "C" void LED_Task(void *argument)
{
    CommandMessage msg;
    bool blinking = false;

    uint8_t currentLedCount = 0;
    uint8_t currentRed = 255;
    uint8_t currentGreen = 255;
    uint8_t currentBlue = 255;

    ledDebug.ledCount = currentLedCount;
    ledDebug.red = currentRed;
    ledDebug.green = currentGreen;
    ledDebug.blue = currentBlue;

    WS2812_SetActiveLeds(currentLedCount,
                         currentRed,
                         currentGreen,
                         currentBlue);

    while (1)
    {
        if (osMessageQueueGet(QueueLedCommandsHandle, &msg, NULL, 0) == osOK)
        {
            switch (msg.id)
            {
                case CMD_ID_START_LED:
                {
                    blinking = true;

                    break;
                }

                case CMD_ID_STOP_LED:
                {
                    blinking = false;

                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

                    /*
                     * STOP LED gasi też pasek WS2812B.
                     */
                    currentLedCount = 0;
                    ledDebug.ledCount = currentLedCount;

                    WS2812_SetActiveLeds(currentLedCount,
                                         currentRed,
                                         currentGreen,
                                         currentBlue);

                    break;
                }

                case CMD_ID_SET_LED:
                {
                    blinking = false;

                    currentRed = msg.led.red;
                    currentGreen = msg.led.green;
                    currentBlue = msg.led.blue;

                    ledDebug.red = currentRed;
                    ledDebug.green = currentGreen;
                    ledDebug.blue = currentBlue;

                    WS2812_SetActiveLeds(currentLedCount,
                                         currentRed,
                                         currentGreen,
                                         currentBlue);

                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);

                    break;
                }

                case CMD_ID_SET_LED_COUNT:
                {
                    blinking = false;

                    currentLedCount = msg.ledCount;

                    ledDebug.ledCount = currentLedCount;

                    WS2812_SetActiveLeds(currentLedCount,
                                         currentRed,
                                         currentGreen,
                                         currentBlue);
                    break;
                }

                default:
                {
                    break;
                }
            }
        }

        if (blinking)
        {
            HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
            osDelay(500);
        }
        else
        {
            osDelay(10);
        }
    }
}
