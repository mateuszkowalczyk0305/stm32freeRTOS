#include "uart_task.h"
#include "RingBuffer.h"
#include "usart.h"
#include "cmsis_os.h"
#include "parser.h"
#include "commands.h"
#include <string.h>
#include <stdio.h>

extern volatile uint16_t photo_adc_raw;
extern volatile uint16_t photo_adc_dark_ref;
extern volatile uint16_t photo_adc_light_ref;
extern volatile uint8_t photo_dark_ref_set;
extern volatile uint8_t photo_light_ref_set;

extern RingBuffer uartBuffer;
extern UART_HandleTypeDef huart2;
extern osMessageQueueId_t QueueLedCommandsHandle;

#define CMD_MAX 32
#define ADC_MAX_VALUE 4095U
#define UART_TX_TIMEOUT_MS 200U

static void uartSendLiteral(const char* text)
{
    HAL_UART_Transmit(&huart2,
                      (uint8_t*)text,
                      (uint16_t)strlen(text),
                      UART_TX_TIMEOUT_MS);
}

static void uartSendFormatted(const char* buffer, int len)
{
    if (len <= 0)
    {
        return;
    }

    uint16_t txLen = (len < (int)80U) ? (uint16_t)len : 79U;

    HAL_UART_Transmit(&huart2,
                      (uint8_t*)buffer,
                      txLen,
                      UART_TX_TIMEOUT_MS);
}

extern "C" void UART_Task(void *argument)
{
    uint8_t c;
    char cmd[CMD_MAX];
    uint8_t idx = 0;

    while (1)
    {
        if (uartBuffer.pop(c))
        {
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
                        uartSendLiteral("Syntax Error\r\n");
                    }
                    else if (msg.id == CMD_ID_SET_DARK_REF)
                    {
                        uint16_t adcNow = photo_adc_raw;
                        char response[80];

                        if (adcNow > ADC_MAX_VALUE)
                        {
                            adcNow = ADC_MAX_VALUE;
                        }

                        photo_adc_dark_ref = adcNow;
                        photo_dark_ref_set = 1U;

                        if (photo_adc_light_ref <= adcNow)
                        {
                            photo_adc_light_ref = (adcNow < ADC_MAX_VALUE) ? (adcNow + 1U) : ADC_MAX_VALUE;
                        }

                        int len = snprintf(response, sizeof(response), "OK DARK SET TO ADC VAL: %u;\r\n", (unsigned int)adcNow);
                        uartSendFormatted(response, len);
                    }
                    else if (msg.id == CMD_ID_SET_LIGHT_REF)
                    {
                        uint16_t adcNow = photo_adc_raw;
                        char response[80];

                        if (adcNow > ADC_MAX_VALUE)
                        {
                            adcNow = ADC_MAX_VALUE;
                        }

                        photo_adc_light_ref = adcNow;
                        photo_light_ref_set = 1U;

                        if (photo_adc_light_ref <= photo_adc_dark_ref)
                        {
                            photo_adc_dark_ref = (photo_adc_light_ref > 0U) ? (photo_adc_light_ref - 1U) : 0U;
                        }

                        int len = snprintf(response, sizeof(response), "OK LIGHT SET TO ADC VAL: %u;\r\n", (unsigned int)adcNow);
                        uartSendFormatted(response, len);
                    }
                    else
                    {
                        osMessageQueuePut(QueueLedCommandsHandle, &msg, 0, 0);
                        uartSendLiteral("OK\r\n");
                    }

                    idx = 0;
                }
            }
            else
            {
                uartSendLiteral("Syntax Error\r\n");
                idx = 0;
            }
        }

        osDelay(1);
    }
}
