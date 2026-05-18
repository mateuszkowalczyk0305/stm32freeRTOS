#include "led_task.h"

#include "cmsis_os.h"
#include "main.h"
#include "gpio.h"
#include "commands.h"
#include "usart.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

extern UART_HandleTypeDef huart2;
extern osMessageQueueId_t QueueLedCommandsHandle;

static_assert(sizeof(CommandMessage) == 12, "CommandMessage size must be 12 bytes");

extern "C" void LED_Task(void *argument)
{
    CommandMessage msg;
    bool blinking = false;

    uint8_t currentLedCount = 0;

    uint8_t currentBrightness = 255;
    uint8_t currentRed = 255;
    uint8_t currentGreen = 255;
    uint8_t currentBlue = 255;

    while (1)
    {
        if (osMessageQueueGet(QueueLedCommandsHandle, &msg, NULL, 0) == osOK)
        {
            switch (msg.id)
            {
                case CMD_ID_START_LED:
                {
                    blinking = true;

                    const char txBuffer[] = "LED blinking START\r\n";

                    HAL_UART_Transmit(&huart2,
                                      (uint8_t*)txBuffer,
                                      strlen(txBuffer),
                                      100);

                    break;
                }

                case CMD_ID_STOP_LED:
                {
                    blinking = false;

                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);

                    const char txBuffer[] = "LED blinking STOP\r\n";

                    HAL_UART_Transmit(&huart2,
                                      (uint8_t*)txBuffer,
                                      strlen(txBuffer),
                                      100);

                    break;
                }

                case CMD_ID_SET_LED:
                {
                    blinking = false;

                    currentBrightness = msg.led.brightness;
                    currentRed = msg.led.red;
                    currentGreen = msg.led.green;
                    currentBlue = msg.led.blue;

                    char txBuffer[120];

                    snprintf(txBuffer, sizeof(txBuffer),
                             "COLOR CMD: brightness=%u R=%u G=%u B=%u | active LEDs=%u\r\n",
                             currentBrightness,
                             currentRed,
                             currentGreen,
                             currentBlue,
                             currentLedCount);

                    HAL_UART_Transmit(&huart2,
                                      (uint8_t*)txBuffer,
                                      strlen(txBuffer),
                                      100);

                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);

                    break;
                }

                case CMD_ID_SET_LED_COUNT:
                {
                    blinking = false;

                    currentLedCount = msg.ledCount;

                    char txBuffer[140];

                    snprintf(txBuffer, sizeof(txBuffer),
                             "PHOTO ADC: %u -> LED count: %u | brightness=%u R=%u G=%u B=%u\r\n",
                             msg.adcRaw,
                             currentLedCount,
                             currentBrightness,
                             currentRed,
                             currentGreen,
                             currentBlue);

                    HAL_UART_Transmit(&huart2,
                                      (uint8_t*)txBuffer,
                                      strlen(txBuffer),
                                      100);

                    break;
                }

                default:
                {
                    const char txBuffer[] = "Unknown LED command\r\n";

                    HAL_UART_Transmit(&huart2,
                                      (uint8_t*)txBuffer,
                                      strlen(txBuffer),
                                      100);

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
