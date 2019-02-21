// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "FreeRTOS.h"
#include "queue.h"
#include "os_Task.h"
#include <chrono>

namespace os
{
template<typename T, size_t n>
class Queue
{
    QueueHandle_t mQueueHandle = nullptr;

public:
    Queue(void);
    Queue(const Queue&) = delete;
    Queue(Queue&&);
    Queue& operator=(const Queue&) = delete;
    Queue& operator=(Queue&&);
    ~Queue(void);

    bool sendFront(T & message, std::chrono::milliseconds) const;
    bool sendFront(T& message, uint32_t ticksToWait = portMAX_DELAY) const;
    bool sendFrontFromISR(T& message) const;
    bool sendFromISR(T& message) const;
    bool sendBack(T & message, std::chrono::milliseconds) const;
    bool sendBack(T& message, uint32_t ticksToWait = portMAX_DELAY) const;
    bool sendBackFromISR(T& message) const;
    bool peek(T & message, std::chrono::milliseconds) const;
    bool peek(T& message, uint32_t ticksToWait = portMAX_DELAY) const;
    bool peekFromISR(T& message) const;
    bool receive(T & message, std::chrono::milliseconds) const;
    bool receive(T& message, uint32_t ticksToWait = portMAX_DELAY) const;
    UBaseType_t messagesWaiting(void) const;
    UBaseType_t spacesAvailable(void) const;
    void reset(void) const;
};

template<typename T>
class Queue<T, 1>
{
    QueueHandle_t mQueueHandle = nullptr;

public:
    Queue(void);
    Queue(const Queue&) = delete;
    Queue(Queue&&);
    Queue& operator=(const Queue&) = delete;
    Queue& operator=(Queue&&);
    ~Queue(void);

    bool peek(T & message, std::chrono::milliseconds = std::chrono::milliseconds(portMAX_DELAY)) const;
    bool peek(T& message, uint32_t ticksToWait = portMAX_DELAY) const;
    bool peekFromISR(T& message) const;
    bool overwrite(const T& message);
    bool overwriteFromISR(const T& message);
    bool receive(T & message, std::chrono::milliseconds = std::chrono::milliseconds(portMAX_DELAY)) const;
    bool receive(T& message, uint32_t ticksToWait = portMAX_DELAY) const;
    void reset(void) const;
};

template<typename T, size_t n>
Queue<T, n>::Queue(void) :
    mQueueHandle(xQueueCreate(n, sizeof(T))) {}

template<typename T, size_t n>
Queue<T, n>::Queue(Queue&& rhs) :
    mQueueHandle(rhs.mQueueHandle)
{
    rhs.mQueueHandle = nullptr;
}

template<typename T, size_t n>
Queue<T, n>& Queue<T, n>::operator=(Queue<T, n>&& rhs)
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
bool Queue<T, n>::sendFront(T& message, std::chrono::milliseconds ticksToWait) const
{
    return xQueueSendToFront(mQueueHandle, &message, ticksToWait.count());
}

template<typename T, size_t n>
bool Queue<T, n>::sendFront(T& message, uint32_t ticksToWait) const
{
    return xQueueSendToFront(mQueueHandle, &message, ticksToWait);
}

template<typename T, size_t n>
bool Queue<T, n>::sendBack(T& message, std::chrono::milliseconds ticksToWait) const
{
    return xQueueSendToBack(mQueueHandle, &message, ticksToWait.count());
}

template<typename T, size_t n>
bool Queue<T, n>::sendBack(T& message, uint32_t ticksToWait) const
{
    return xQueueSendToBack(mQueueHandle, &message, ticksToWait);
}

template<typename T, size_t n>
bool Queue<T, n>::sendFrontFromISR(T& message) const
{
    BaseType_t highPriorityTaskWoken = 0;

    bool retValue = xQueueSendToFrontFromISR(mQueueHandle, &message, &highPriorityTaskWoken);
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retValue;
}

template<typename T, size_t n>
bool Queue<T, n>::sendBackFromISR(T& message) const
{
    BaseType_t highPriorityTaskWoken = 0;

    bool retValue = xQueueSendToBackFromISR(mQueueHandle, &message, &highPriorityTaskWoken);
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retValue;
}

template<typename T, size_t n>
bool Queue<T, n>::sendFromISR(T& message) const
{
    BaseType_t highPriorityTaskWoken = 0;

    bool retValue = xQueueSendFromISR(mQueueHandle, &message, &highPriorityTaskWoken);
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retValue;
}

template<typename T, size_t n>
bool Queue<T, n>::peek(T& message, std::chrono::milliseconds ticksToWait) const
{
    return xQueuePeek(mQueueHandle, &message, ticksToWait.count());
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
bool Queue<T, n>::receive(T& message, std::chrono::milliseconds ticksToWait) const
{
    return xQueueReceive(mQueueHandle, &message, ticksToWait.count());
}

template<typename T, size_t n>
bool Queue<T, n>::receive(T& message, uint32_t ticksToWait) const
{
    return xQueueReceive(mQueueHandle, &message, ticksToWait);
}

template<typename T, size_t n>
UBaseType_t Queue<T, n>::messagesWaiting(void) const
{
    return uxQueueMessagesWaiting(mQueueHandle);
}

template<typename T, size_t n>
UBaseType_t Queue<T, n>::spacesAvailable(void) const
{
    return uxQueueSpacesAvailable(mQueueHandle);
}

template<typename T, size_t n>
void Queue<T, n>::reset(void) const
{
    xQueueReset(mQueueHandle);
}
///////////////////////////////////////////////////////////

template<typename T>
Queue<T, 1>::Queue(void) :
    mQueueHandle(xQueueCreate(1, sizeof(T)))
{}

template<typename T>
Queue<T, 1>::Queue(Queue<T, 1>&& rhs) :
    mQueueHandle(rhs.mQueueHandle)
{
    rhs.mQueueHandle = nullptr;
}

template<typename T>
Queue<T, 1>& Queue<T, 1>::operator=(Queue<T, 1>&& rhs)
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
bool Queue<T, 1>::peek(T& message, std::chrono::milliseconds ticksToWait) const
{
    return xQueuePeek(mQueueHandle, &message, ticksToWait.count());
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
bool Queue<T, 1>::overwrite(const T& message)
{
    return xQueueOverwrite(mQueueHandle, &message);
}

template<typename T>
bool Queue<T, 1>::overwriteFromISR(const T& message)
{
    BaseType_t highPriorityTaskWoken = 0;

    bool retValue = xQueueOverwriteFromISR(mQueueHandle, &message, &highPriorityTaskWoken);
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retValue;
}

template<typename T>
bool Queue<T, 1>::receive(T& message, std::chrono::milliseconds ticksToWait) const
{
    return xQueueReceive(mQueueHandle, &message, ticksToWait.count());
}

template<typename T>
bool Queue<T, 1>::receive(T& message, uint32_t ticksToWait) const
{
    return xQueueReceive(mQueueHandle, &message, ticksToWait);
}

template<typename T>
void Queue<T, 1>::reset(void) const
{
    xQueueReset(mQueueHandle);
}
}
