#include "RingBuffer.h"


void RingBuffer::init()
{
    head = 0;
    tail = 0;
}

bool RingBuffer::isEmpty() const
{
    return head == tail;
}

bool RingBuffer::isFull() const
{
    return ((head + 1) % RING_BUFFER_SIZE) == tail;
}

bool RingBuffer::push(uint8_t data)
{
    uint16_t next = (head + 1) % RING_BUFFER_SIZE;

    if (next == tail)
        return false; // overflow

    buffer[head] = data;
    head = next;
    return true;
}

bool RingBuffer::pop(uint8_t &data)
{
    if (isEmpty())
        return false;

    data = buffer[tail];
    tail = (tail + 1) % RING_BUFFER_SIZE;
    return true;
}
