#pragma once

#ifndef INC_BSP_UART_H_
#define INC_BSP_UART_H_

#include "stm32f3xx_hal.h"

class RingBuffer;

void BSP_UART_Init(UART_HandleTypeDef *huart, RingBuffer* buffer);

#endif /* INC_BSP_UART_H_ */
