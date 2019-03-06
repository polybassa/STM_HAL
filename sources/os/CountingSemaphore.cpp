// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "CountingSemaphore.h"

using os::CountingSemaphore;

CountingSemaphore::CountingSemaphore(uint32_t maximalCount,
                                     uint32_t initalCount) :
    mSemaphoreHandle(xSemaphoreCreateCounting(
                                              maximalCount,
                                              initalCount))
{}

CountingSemaphore::CountingSemaphore(CountingSemaphore&& rhs) :
    mSemaphoreHandle(rhs.mSemaphoreHandle)
{
    rhs.mSemaphoreHandle = nullptr;
}

CountingSemaphore& CountingSemaphore::operator=(CountingSemaphore&& rhs)
{
    mSemaphoreHandle = rhs.mSemaphoreHandle;
    rhs.mSemaphoreHandle = nullptr;
    return *this;
}

CountingSemaphore::~CountingSemaphore(void)
{
    if (*this) {
        vSemaphoreDelete(mSemaphoreHandle);
    }
}

bool CountingSemaphore::take(uint32_t ticksToWait) const
{
    return *this ? xSemaphoreTake(mSemaphoreHandle, ticksToWait) : false;
}

bool CountingSemaphore::give(void) const
{
    return *this ? xSemaphoreGive(mSemaphoreHandle) : false;
}

bool CountingSemaphore::giveFromISR(void) const
{
    BaseType_t highPriorityTaskWoken = 0;
    bool retVal = *this ? xSemaphoreGiveFromISR(mSemaphoreHandle, &highPriorityTaskWoken) : false;
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retVal;
}

bool CountingSemaphore::takeFromISR(void) const
{
    BaseType_t highPriorityTaskWoken = 0;
    bool retVal = *this ? xSemaphoreTakeFromISR(mSemaphoreHandle, &highPriorityTaskWoken) : false;
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retVal;
}

CountingSemaphore::operator bool() const
{
    return mSemaphoreHandle != nullptr;
}
