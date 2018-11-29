// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "Mutex.h"

using os::Mutex;

Mutex::Mutex(void) :
    mMutexHandle(xSemaphoreCreateMutex())
{}

Mutex::Mutex(Mutex && rhs) :
    mMutexHandle(rhs.mMutexHandle)
{
    rhs.mMutexHandle = nullptr;
}

Mutex& Mutex::operator=(Mutex && rhs)
{
    mMutexHandle = rhs.mMutexHandle;
    rhs.mMutexHandle = nullptr;
    return *this;
}

Mutex::~Mutex(void)
{
    if (*this) {
        vSemaphoreDelete(mMutexHandle);
    }
}

bool Mutex::take(uint32_t ticksToWait) const
{
    return *this ? xSemaphoreTake(mMutexHandle, ticksToWait) : false;
}

bool Mutex::give(void) const
{
    return *this ? xSemaphoreGive(mMutexHandle) : false;
}

Mutex::operator bool() const
{
    return mMutexHandle != nullptr;
}
