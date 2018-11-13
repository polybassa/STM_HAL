// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "RecursiveMutex.h"

using os::RecursiveMutex;

RecursiveMutex::RecursiveMutex(void) :
    mMutexHandle(xSemaphoreCreateRecursiveMutex())
{}

RecursiveMutex::RecursiveMutex(RecursiveMutex&& rhs) :
    mMutexHandle(rhs.mMutexHandle)
{
    rhs.mMutexHandle = nullptr;
}

RecursiveMutex& RecursiveMutex::operator=(RecursiveMutex&& rhs)
{
    mMutexHandle = rhs.mMutexHandle;
    rhs.mMutexHandle = nullptr;
    return *this;
}

RecursiveMutex::~RecursiveMutex(void)
{
    vSemaphoreDelete(mMutexHandle);
}

bool RecursiveMutex::take(uint32_t ticksToWait) const
{
    return *this ? xSemaphoreTakeRecursive(mMutexHandle, ticksToWait) : false;
}

bool RecursiveMutex::give(void) const
{
    return *this ? xSemaphoreGiveRecursive(mMutexHandle) : false;
}

RecursiveMutex::operator bool() const
{
    return mMutexHandle != nullptr;
}
