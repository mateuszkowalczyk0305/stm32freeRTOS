#include "uart_task.h"
#include "RingBuffer.h"
#include "usart.h"
#include "cmsis_os.h"
#include "parser.h"
#include "commands.h"
#include <string.h>

extern RingBuffer uartBuffer;
extern UART_HandleTypeDef huart2;
extern osMessageQueueId_t QueueLedCommandsHandle;

#define CMD_MAX 32

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
                if (idx == 0)
                    continue;

                cmd[idx] = '\0';

                CommandMessage msg = parseCommand(cmd);

                if (msg.id == CMD_ID_UNKNOWN)
                {
                    HAL_UART_Transmit(&huart2, (uint8_t*)"Syntax Error\r\n", 14, 10);
                }
                else
                {
                    osMessageQueuePut(QueueLedCommandsHandle, &msg, 0, 0);

                    HAL_UART_Transmit(&huart2, (uint8_t*)"OK\r\n", 4, 10);
                }

                idx = 0;
            }
            else
            {
                if (idx < CMD_MAX - 1)
                {
                    cmd[idx++] = c;
                }
                else
                {
                    HAL_UART_Transmit(&huart2, (uint8_t*)"Syntax Error\r\n", 14, 10);
                    idx = 0;
                }
            }
        }

        osDelay(1);
    }
}
