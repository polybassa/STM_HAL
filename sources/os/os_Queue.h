/* Copyright (C) 2015  Nils Weiss
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef SOURCES_PMD__QUEUE_H_
#define SOURCES_PMD__QUEUE_H_

#include "FreeRTOS.h"
#include "queue.h"

namespace os
{
template<typename T, size_t n>
class Queue
{
    QueueHandle_t mQueueHandle = nullptr;

public:
    Queue(void);
    Queue(const Queue&) = delete;
    Queue(Queue &&);
    Queue& operator=(const Queue&) = delete;
    Queue& operator=(Queue &&);
    ~Queue(void);

    bool sendFront(T message, uint32_t ticksToWait = portMAX_DELAY) const;
    bool sendFrontFromISR(T message) const;
    bool sendBack(T message, uint32_t ticksToWait = portMAX_DELAY) const;
    bool sendBackFromISR(T message) const;
    bool peek(T& message, uint32_t ticksToWait = portMAX_DELAY) const;
    bool peekFromISR(T& message) const;
    bool receive(T& message, uint32_t ticksToWait = portMAX_DELAY) const;
    uint32_t messagesWaiting(void) const;
    uint32_t spacesAvailable(void) const;
    void reset(void) const;
};

template<typename T>
class Queue<T, 1>
{
    QueueHandle_t mQueueHandle = nullptr;

public:
    Queue(void);
    Queue(const Queue&) = delete;
    Queue(Queue &&);
    Queue& operator=(const Queue&) = delete;
    Queue& operator=(Queue &&);
    ~Queue(void);

    bool peek(T& message, uint32_t ticksToWait = portMAX_DELAY) const;
    bool peekFromISR(T& message) const;
    bool overwrite(const T message);
    bool overwriteFromISR(const T message);
    bool receive(T& message, uint32_t ticksToWait = portMAX_DELAY) const;
    void reset(void) const;
};

template<typename T, size_t n>
Queue<T, n>::Queue(void) :
    mQueueHandle(xQueueCreate(n, sizeof(T))) {}

template<typename T, size_t n>
Queue<T, n>::Queue(Queue && rhs) :
    mQueueHandle(rhs.mQueueHandle)
{
    rhs.mQueueHandle = nullptr;
}

template<typename T, size_t n>
Queue<T, n>& Queue<T, n>::operator=(Queue<T, n> && rhs)
{
    mQueueHandle = rhs.mQueueHandle;
    rhs.mQueueHandle = nullptr;
    return *this;
}

template<typename T, size_t n>
Queue<T, n>::~Queue(void)
{
    vQueueDelete(mQueueHandle);
}

template<typename T, size_t n>
bool Queue<T, n>::sendFront(T message, uint32_t ticksToWait) const
{
    return xQueueSendToFront(mQueueHandle, &message, ticksToWait);
}

template<typename T, size_t n>
bool Queue<T, n>::sendBack(T message, uint32_t ticksToWait) const
{
    return xQueueSendToBack(mQueueHandle, &message, ticksToWait);
}

template<typename T, size_t n>
bool Queue<T, n>::sendFrontFromISR(T message) const
{
    uint32_t highPriorityTaskWoken = 0;

    bool retValue = xQueueSendToFrontFromISR(mQueueHandle, &message, &highPriorityTaskWoken);
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retValue;
}

template<typename T, size_t n>
bool Queue<T, n>::sendBackFromISR(T message) const
{
    uint32_t highPriorityTaskWoken = 0;

    bool retValue = xQueueSendToBackFromISR(mQueueHandle, &message, &highPriorityTaskWoken);
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retValue;
}

template<typename T, size_t n>
bool Queue<T, n>::peek(T& message, uint32_t ticksToWait) const
{
    return xQueuePeek(mQueueHandle, &message, ticksToWait);
}

template<typename T, size_t n>
bool Queue<T, n>::peekFromISR(T& message) const
{
    return xQueuePeekFromISR(mQueueHandle, &message);
}

template<typename T, size_t n>
bool Queue<T, n>::receive(T& message, uint32_t ticksToWait) const
{
    return xQueueReceive(mQueueHandle, &message, ticksToWait);
}

template<typename T, size_t n>
uint32_t Queue<T, n>::messagesWaiting(void) const
{
    return uxQueueMessagesWaiting(mQueueHandle);
}

template<typename T, size_t n>
uint32_t Queue<T, n>::spacesAvailable(void) const
{
    return uxQueueSpacesAvailable(mQueueHandle);
}

template<typename T, size_t n>
void Queue<T, n>::reset(void) const
{
    return xQueueReset(mQueueHandle);
}
///////////////////////////////////////////////////////////

template<typename T>
Queue<T, 1>::Queue(void) :
    mQueueHandle(xQueueCreate(1, sizeof(T)))
{}

template<typename T>
Queue<T, 1>::Queue(Queue<T, 1> && rhs) :
    mQueueHandle(rhs.mQueueHandle)
{
    rhs.mQueueHandle = nullptr;
}

template<typename T>
Queue<T, 1>& Queue<T, 1>::operator=(Queue<T, 1> && rhs)
{
    mQueueHandle = rhs.mQueueHandle;
    rhs.mQueueHandle = nullptr;
    return *this;
}

template<typename T>
Queue<T, 1>::~Queue(void)
{
    vQueueDelete(mQueueHandle);
}

template<typename T>
bool Queue<T, 1>::peek(T& message, uint32_t ticksToWait) const
{
    return xQueuePeek(mQueueHandle, &message, ticksToWait);
}

template<typename T>
bool Queue<T, 1>::peekFromISR(T& message) const
{
    return xQueuePeekFromISR(mQueueHandle, &message);
}

template<typename T>
bool Queue<T, 1>::overwrite(const T message)
{
    return xQueueOverwrite(mQueueHandle, &message);
}

template<typename T>
bool Queue<T, 1>::overwriteFromISR(T message)
{
    uint32_t highPriorityTaskWoken = 0;

    bool retValue = xQueueOverwriteFromISR(mQueueHandle, &message, &highPriorityTaskWoken);
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retValue;
}

template<typename T>
bool Queue<T, 1>::receive(T& message, uint32_t ticksToWait) const
{
    return xQueueReceive(mQueueHandle, &message, ticksToWait);
}

template<typename T>
void Queue<T, 1>::reset(void) const
{
    return xQueueReset(mQueueHandle);
}
}

#endif /* SOURCES_PMD__QUEUE_H_ */
