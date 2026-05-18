#include "led_task.h"

#include "cmsis_os.h"
#include "main.h"
#include "gpio.h"
#include "commands.h"
#include "usart.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

extern UART_HandleTypeDef huart2;
extern osMessageQueueId_t QueueLedCommandsHandle;

extern "C" void LED_Task(void *argument)
{
    CommandMessage msg;
    bool blinking = false;

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

                    char txBuffer[100];

                    snprintf(txBuffer, sizeof(txBuffer),
                             "LED CMD: led=%u brightness=%u R=%u G=%u B=%u\r\n",
                             msg.led.ledNumber,
                             msg.led.brightness,
                             msg.led.red,
                             msg.led.green,
                             msg.led.blue);

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

                    char txBuffer[100];

                    snprintf(txBuffer, sizeof(txBuffer),
                             "PHOTO ADC: %u -> LED count: %u\r\n",
                             msg.adcRaw,
                             msg.ledCount);

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
