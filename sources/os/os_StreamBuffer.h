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

#ifndef SOURCES_PMD__STREAMBUFFER_H_
#define SOURCES_PMD__STREAMBUFFER_H_

#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "os_Task.h"

namespace os
{
template<typename T, size_t n>
class StreamBuffer
{
    StreamBufferHandle_t mStreamBufferHandle = nullptr;

public:
    StreamBuffer(const size_t triggerLevel = 1);
    StreamBuffer(const StreamBuffer&) = delete;
    StreamBuffer(StreamBuffer &&);
    StreamBuffer& operator=(const StreamBuffer&) = delete;
    StreamBuffer& operator=(StreamBuffer &&);
    ~StreamBuffer(void);

    bool send(T message, uint32_t ticksToWait = portMAX_DELAY) const;
    bool sendFromISR(T message) const;
    bool receive(T& message, uint32_t ticksToWait = portMAX_DELAY) const;
    bool receive(char* message, const size_t length, uint32_t ticksToWait = portMAX_DELAY) const;
    bool receiveFromISR(T& message) const;

    bool isFull(void) const;
    bool isEmpty(void) const;
    bool reset(void) const;

    size_t spacesAvailable(void) const;
    size_t bytesAvailable(void) const;

    bool setTriggerLevel(size_t triggerLevel) const;

    bool sendCompletedFromISR(void) const;
    bool receiveCompletedFromISR(void) const;
};

template<typename T, size_t n>
StreamBuffer<T, n>::StreamBuffer(const size_t triggerLevel) :
    mStreamBufferHandle(xStreamBufferCreate(n * sizeof(T), triggerLevel)) {}

template<typename T, size_t n>
StreamBuffer<T, n>::StreamBuffer(StreamBuffer && rhs) :
    mStreamBufferHandle(rhs.mStreamBufferHandle)
{
    rhs.mStreamBufferHandle = nullptr;
}

template<typename T, size_t n>
StreamBuffer<T, n>& StreamBuffer<T, n>::operator=(StreamBuffer<T, n> && rhs)
{
    mStreamBufferHandle = rhs.mStreamBufferHandle;
    rhs.mStreamBufferHandle = nullptr;
    return *this;
}

template<typename T, size_t n>
StreamBuffer<T, n>::~StreamBuffer(void)
{
    vStreamBufferDelete(mStreamBufferHandle);
}

template<typename T, size_t n>
bool StreamBuffer<T, n>::send(T message, uint32_t ticksToWait) const
{
    return xStreamBufferSend(mStreamBufferHandle, &message, sizeof(message), ticksToWait) == sizeof(message);
}

template<typename T, size_t n>
bool StreamBuffer<T, n>::sendFromISR(T message) const
{
    BaseType_t highPriorityTaskWoken = 0;

    auto retValue = xStreamBufferSendFromISR(mStreamBufferHandle, &message, sizeof(message), &highPriorityTaskWoken);
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retValue == sizeof(message);
}

template<typename T, size_t n>
bool StreamBuffer<T, n>::receive(char* message, const size_t length, uint32_t ticksToWait) const
{
    return xStreamBufferReceive(mStreamBufferHandle, message, length, ticksToWait) == length;
}

template<typename T, size_t n>
bool StreamBuffer<T, n>::receive(T& message, uint32_t ticksToWait) const
{
    return xStreamBufferReceive(mStreamBufferHandle, &message, sizeof(message), ticksToWait) == sizeof(message);
}

template<typename T, size_t n>
bool StreamBuffer<T, n>::receiveFromISR(T& message) const
{
    uint32_t highPriorityTaskWoken = 0;

    bool retValue = xStreamBufferReceiveFromISR(mStreamBufferHandle, &message, sizeof(message), &highPriorityTaskWoken);
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retValue == sizeof(message);
}

template<typename T, size_t n>
bool StreamBuffer<T, n>::isFull(void) const
{
    return xStreamBufferIsFull(mStreamBufferHandle) == pdTRUE;
}

template<typename T, size_t n>
bool StreamBuffer<T, n>::isEmpty(void) const
{
    return xStreamBufferIsEmpty(mStreamBufferHandle) == pdTRUE;
}

template<typename T, size_t n>
bool StreamBuffer<T, n>::reset(void) const
{
    return xStreamBufferReset(mStreamBufferHandle) == pdTRUE;
}

template<typename T, size_t n>
size_t StreamBuffer<T, n>::spacesAvailable(void) const
{
    return xStreamBufferSpacesAvailable(mStreamBufferHandle);
}

template<typename T, size_t n>
size_t StreamBuffer<T, n>::bytesAvailable(void) const
{
    return xStreamBufferBytesAvailable(mStreamBufferHandle);
}

template<typename T, size_t n>
bool StreamBuffer<T, n>::setTriggerLevel(size_t triggerLevel) const
{
    return xStreamBufferSetTriggerLevel(mStreamBufferHandle, triggerLevel) == pdTRUE;
}

template<typename T, size_t n>
bool StreamBuffer<T, n>::sendCompletedFromISR(void) const
{
    BaseType_t highPriorityTaskWoken = 0;

    auto retValue = xStreamBufferSendCompletedFromISR(mStreamBufferHandle, &highPriorityTaskWoken);
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retValue == pdTRUE;
}

template<typename T, size_t n>
bool StreamBuffer<T, n>::receiveCompletedFromISR(void) const
{
    BaseType_t highPriorityTaskWoken = 0;

    auto retValue = xStreamBufferReceiveCompletedFromISR(mStreamBufferHandle, &highPriorityTaskWoken);
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retValue == pdTRUE;
}
}
#endif /* SOURCES_PMD__STREAMBUFFER_H_ */
