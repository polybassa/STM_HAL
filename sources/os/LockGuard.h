// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

namespace os
{
template<typename T>
class LockGuard
{
public:

    explicit LockGuard(const T& lock, portTickType ticksToWait = portMAX_DELAY) :
        mLock(lock)
    {
        if (mLock) {
            this->mLockObtained = mLock.take(ticksToWait);
        }
    }

    ~LockGuard()
    {
        if (mLock) {
            mLock.give();
        }
    }

    LockGuard(const LockGuard&) = delete;
    LockGuard& operator=(const LockGuard&) = delete;

    operator bool(void) const {
        return mLockObtained;
    }

private:
    const T& mLock;
    bool mLockObtained = false;
};
}
