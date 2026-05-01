/*
 * RingBuffer.h
 *
 *  Created on: May 1, 2026
 *      Author: m_kowalczyk
 */

#pragma once

#ifndef INC_RINGBUFFER_H_
#define INC_RINGBUFFER_H_

#include <stdint.h>
#include <stdbool.h>

#define RING_BUFFER_SIZE 128

class RingBuffer
{
public:
    void init();

    bool push(uint8_t data);   // ISR
    bool pop(uint8_t &data);   // task

    bool isEmpty() const;
    bool isFull() const;

private:
    volatile uint8_t buffer[RING_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
};


#endif /* INC_RINGBUFFER_H_ */
