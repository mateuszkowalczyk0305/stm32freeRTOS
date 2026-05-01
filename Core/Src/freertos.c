/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "commands.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for TaskUartCmd */
osThreadId_t TaskUartCmdHandle;
const osThreadAttr_t TaskUartCmd_attributes = {
  .name = "TaskUartCmd",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for TaskLedOper */
osThreadId_t TaskLedOperHandle;
const osThreadAttr_t TaskLedOper_attributes = {
  .name = "TaskLedOper",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for QueueLedCommands */
osMessageQueueId_t QueueLedCommandsHandle;
const osMessageQueueAttr_t QueueLedCommands_attributes = {
  .name = "QueueLedCommands"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void TFuncUartCmd(void *argument);
void TFuncLedOper(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of QueueLedCommands */
  QueueLedCommandsHandle = osMessageQueueNew (8, sizeof(CommandID), &QueueLedCommands_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of TaskUartCmd */
  TaskUartCmdHandle = osThreadNew(TFuncUartCmd, NULL, &TaskUartCmd_attributes);

  /* creation of TaskLedOper */
  TaskLedOperHandle = osThreadNew(TFuncLedOper, NULL, &TaskLedOper_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_TFuncUartCmd */
/**
  * @brief  Function implementing the TaskUartCmd thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_TFuncUartCmd */
void TFuncUartCmd(void *argument)
{
  /* USER CODE BEGIN TFuncUartCmd */
  /* Wrapper */
  extern void UART_Task(void *argument);
  UART_Task(argument);
  /* USER CODE END TFuncUartCmd */
}

/* USER CODE BEGIN Header_TFuncLedOper */
/**
* @brief Function implementing the TaskLedOper thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_TFuncLedOper */
void TFuncLedOper(void *argument)
{
  /* USER CODE BEGIN TFuncLedOper */

	/* Wrapper */
	extern void LED_Task(void *argument);
	LED_Task(argument);
  /* USER CODE END TFuncLedOper */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

