/*
 * bsp_uart.cpp
 *
 *  Created on: May 1, 2026
 *      Author: m_kowalczyk
 */

#include "bsp_uart.h"
#include "RingBuffer.h"

extern UART_HandleTypeDef huart2;
static uint8_t rxByte;
static RingBuffer* uartBufferPtr = nullptr;

void BSP_UART_Init(UART_HandleTypeDef *huart, RingBuffer* buffer)
{
    uartBufferPtr = buffer;
    HAL_UART_Receive_IT(huart, &rxByte, 1);
}

/* HAL callback (extern "C" - necessary) */
extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	 if (huart->Instance == USART2)
	    {
	        if (uartBufferPtr)
	        {
	            uartBufferPtr->push(rxByte);
	        }

	        HAL_UART_Receive_IT(&huart2, &rxByte, 1);
	    }
}

