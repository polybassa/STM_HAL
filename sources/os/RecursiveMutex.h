// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "FreeRTOS.h"
#include "semphr.h"
#include <chrono>

namespace os
{
class RecursiveMutex
{
    SemaphoreHandle_t mMutexHandle = nullptr;
    bool take(const uint32_t ticksToWait) const;

public:
    RecursiveMutex(void);
    RecursiveMutex(const RecursiveMutex&) = delete;
    RecursiveMutex(RecursiveMutex&&);
    RecursiveMutex& operator=(const RecursiveMutex&) = delete;
    RecursiveMutex& operator=(RecursiveMutex&&);
    ~RecursiveMutex(void);

    inline bool take(void) const {return this->take(portMAX_DELAY); }

    template<class rep, class period>
    inline bool take(const std::chrono::duration<rep, period>& d) const
    {
        return take(std::chrono::duration_cast<std::chrono::milliseconds>(d).count() / portTICK_RATE_MS);
    }

    bool give(void) const;

    operator bool() const;
};
}
