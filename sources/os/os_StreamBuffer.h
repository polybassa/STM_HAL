// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "os_Task.h"

namespace os
{
template<typename T, size_t n>
class StreamBuffer
{
    StreamBufferHandle_t mStreamBufferHandle = nullptr;

    bool send(T message, uint32_t ticksToWait) const;
    size_t send(T const* message, const size_t length, uint32_t ticksToWait) const;
    bool receive(T& message, const uint32_t ticksToWait) const;
    size_t receive(T* message, const size_t length, const uint32_t ticksToWait) const;

public:
    StreamBuffer(const size_t triggerLevel = 1);
    StreamBuffer(const StreamBuffer&) = delete;
    StreamBuffer(StreamBuffer&&);
    StreamBuffer& operator=(const StreamBuffer&) = delete;
    StreamBuffer& operator=(StreamBuffer&&);
    ~StreamBuffer(void);

    template<class rep, class period>
    inline bool send(const T message, const std::chrono::duration<rep, period>& d) const
    {
        return send(message, std::chrono::duration_cast<std::chrono::milliseconds>(d).count() / portTICK_RATE_MS);
    }
    inline bool send(const T message) const {return send(message, portMAX_DELAY);}

    template<class rep, class period>
    inline size_t send(T const* message, const size_t length, const std::chrono::duration<rep, period>& d) const
    {
        return send(message, length,
                    std::chrono::duration_cast<std::chrono::milliseconds>(d).count() / portTICK_RATE_MS);
    }
    inline size_t send(T const* message, const size_t length) const { return send(message, length, portMAX_DELAY);}

    bool sendFromISR(const T message) const;

    template<class rep, class period>
    inline bool receive(T& message, const std::chrono::duration<rep, period>& d) const
    {
        return receive(message, std::chrono::duration_cast<std::chrono::milliseconds>(d).count() / portTICK_RATE_MS);
    }
    inline bool receive(T& message) const {return receive(message, portMAX_DELAY);}

    template<class rep, class period>
    inline size_t receive(T* message, const size_t length, const std::chrono::duration<rep, period>& d) const
    {
        return receive(message, length,
                       std::chrono::duration_cast<std::chrono::milliseconds>(d).count() / portTICK_RATE_MS);
    }
    inline size_t receive(T* message, const size_t length) const {return receive(message, length, portMAX_DELAY);}

    bool receiveFromISR(T& message) const;

    bool isFull(void) const;
    bool isEmpty(void) const;
    bool reset(void) const;

    size_t spacesAvailable(void) const;
    size_t bytesAvailable(void) const;

    bool setTriggerLevel(const size_t triggerLevel) const;

    bool sendCompletedFromISR(void) const;
    bool receiveCompletedFromISR(void) const;
};

template<typename T, size_t n>
StreamBuffer<T, n>::StreamBuffer(const size_t triggerLevel) :
    mStreamBufferHandle(xStreamBufferCreate(n * sizeof(T), triggerLevel)) {}

template<typename T, size_t n>
StreamBuffer<T, n>::StreamBuffer(StreamBuffer&& rhs) :
    mStreamBufferHandle(rhs.mStreamBufferHandle)
{
    rhs.mStreamBufferHandle = nullptr;
}

template<typename T, size_t n>
StreamBuffer<T, n>& StreamBuffer<T, n>::operator=(StreamBuffer<T, n>&& rhs)
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
bool StreamBuffer<T, n>::send(const T message, const uint32_t ticksToWait) const
{
    return xStreamBufferSend(mStreamBufferHandle, &message, sizeof(message), ticksToWait) == sizeof(message);
}
template<typename T, size_t n>
size_t StreamBuffer<T, n>::send(T const* message, const size_t length, const uint32_t ticksToWait) const
{
    return xStreamBufferSend(mStreamBufferHandle, message, length, ticksToWait);
}

template<typename T, size_t n>
bool StreamBuffer<T, n>::sendFromISR(const T message) const
{
    BaseType_t highPriorityTaskWoken = 0;

    auto retValue = xStreamBufferSendFromISR(mStreamBufferHandle, &message, sizeof(message), &highPriorityTaskWoken);
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retValue == sizeof(message);
}

template<typename T, size_t n>
size_t StreamBuffer<T, n>::receive(T* message, const size_t length, const uint32_t ticksToWait) const
{
    return xStreamBufferReceive(mStreamBufferHandle, message, length, ticksToWait);
}

template<typename T, size_t n>
bool StreamBuffer<T, n>::receive(T& message, const uint32_t ticksToWait) const
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
bool StreamBuffer<T, n>::setTriggerLevel(const size_t triggerLevel) const
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
