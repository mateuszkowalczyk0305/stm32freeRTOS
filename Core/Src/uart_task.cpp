#include "uart_task.h"
#include "RingBuffer.h"
#include "usart.h"
#include "cmsis_os.h"
#include "parser.h"
#include "commands.h"
#include <string.h>

extern volatile uint16_t photo_adc_raw;
extern volatile uint16_t photo_adc_dark_ref;
extern volatile uint16_t photo_adc_light_ref;

extern RingBuffer uartBuffer;
extern UART_HandleTypeDef huart2;
extern osMessageQueueId_t QueueLedCommandsHandle;

#define CMD_MAX 32
#define ADC_MAX_VALUE 4095U

extern "C" void UART_Task(void *argument)
{
    uint8_t c;
    char cmd[CMD_MAX];
    uint8_t idx = 0;

    while (1)
    {
        if (uartBuffer.pop(c))
        {
            // Echo odebranego znaku
            HAL_UART_Transmit(&huart2, &c, 1, 10);

            if (c == '\n' || c == '\r')
            {
                continue;
            }

            if (idx < CMD_MAX - 1)
            {
                cmd[idx++] = c;

                if (c == ';')
                {
                    cmd[idx] = '\0';

                    CommandMessage msg = parseCommand(cmd);

                    if (msg.id == CMD_ID_UNKNOWN)
                    {
                        HAL_UART_Transmit(&huart2, (uint8_t*)"Syntax Error\r\n", 14, 10);
                    }
                    else if (msg.id == CMD_ID_SET_DARK_REF)
                    {
                        uint16_t adcNow = photo_adc_raw;

                        if (adcNow > ADC_MAX_VALUE)
                        {
                            adcNow = ADC_MAX_VALUE;
                        }

                        photo_adc_dark_ref = adcNow;

                        if (photo_adc_light_ref <= adcNow)
                        {
                            photo_adc_light_ref = (adcNow < ADC_MAX_VALUE) ? (adcNow + 1U) : ADC_MAX_VALUE;
                        }

                        HAL_UART_Transmit(&huart2, (uint8_t*)"OK SD\r\n", 7, 10);
                    }
                    else if (msg.id == CMD_ID_SET_LIGHT_REF)
                    {
                        uint16_t adcNow = photo_adc_raw;

                        if (adcNow > ADC_MAX_VALUE)
                        {
                            adcNow = ADC_MAX_VALUE;
                        }

                        photo_adc_light_ref = adcNow;

                        if (photo_adc_light_ref <= photo_adc_dark_ref)
                        {
                            photo_adc_dark_ref = (photo_adc_light_ref > 0U) ? (photo_adc_light_ref - 1U) : 0U;
                        }

                        HAL_UART_Transmit(&huart2, (uint8_t*)"OK SL\r\n", 7, 10);
                    }
                    else
                    {
                        osMessageQueuePut(QueueLedCommandsHandle, &msg, 0, 0);
                        HAL_UART_Transmit(&huart2, (uint8_t*)"OK\r\n", 4, 10);
                    }

                    idx = 0;
                }
            }
            else
            {
                HAL_UART_Transmit(&huart2, (uint8_t*)"Syntax Error\r\n", 14, 10);
                idx = 0;
            }
        }

        osDelay(1);
    }
}
