#include "led_task.h"
#include "gpio.h"
#include "cmsis_os.h"
#include "commands.h"

extern osMessageQueueId_t QueueLedCommandsHandle;

extern "C" void LED_Task(void *argument)
{
    CommandID cmd;
    uint8_t blinking = 0;

    while (1)
    {
        // sprawdz czy przyszla komenda
        if (osMessageQueueGet(QueueLedCommandsHandle, &cmd, NULL, 0) == osOK)
        {
            switch (cmd)
            {
                case CMD_ID_START_LED:
                    blinking = 1;
                    break;

                case CMD_ID_STOP_LED:
                    blinking = 0;
                    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
                    break;

                default:
                    break;
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
