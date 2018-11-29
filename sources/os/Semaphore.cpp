// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "Semaphore.h"
#include "os_Task.h"

using os::Semaphore;

Semaphore::Semaphore(void) :
    mSemaphoreHandle(xSemaphoreCreateBinary())
{}

Semaphore::Semaphore(Semaphore && rhs) :
    mSemaphoreHandle(rhs.mSemaphoreHandle)
{
    rhs.mSemaphoreHandle = nullptr;
}

Semaphore& Semaphore::operator=(Semaphore && rhs)
{
    mSemaphoreHandle = rhs.mSemaphoreHandle;
    rhs.mSemaphoreHandle = nullptr;
    return *this;
}

Semaphore::~Semaphore(void)
{
    if (*this) {
        vSemaphoreDelete(mSemaphoreHandle);
    }
}

bool Semaphore::take(const uint32_t ticksToWait) const
{
    return *this ? xSemaphoreTake(mSemaphoreHandle, ticksToWait) : false;
}

bool Semaphore::give(void) const
{
    return *this ? xSemaphoreGive(mSemaphoreHandle) : false;
}

bool Semaphore::giveFromISR(void) const
{
    BaseType_t highPriorityTaskWoken = 0;
    bool retVal = *this ? xSemaphoreGiveFromISR(mSemaphoreHandle, &highPriorityTaskWoken) : false;
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retVal;
}

bool Semaphore::takeFromISR(void) const
{
    BaseType_t highPriorityTaskWoken = 0;
    bool retVal = *this ? xSemaphoreTakeFromISR(mSemaphoreHandle, &highPriorityTaskWoken) : false;
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retVal;
}

Semaphore::operator bool() const
{
    return mSemaphoreHandle != nullptr;
}
