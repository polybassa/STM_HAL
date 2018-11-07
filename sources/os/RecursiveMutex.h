// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

namespace os
{
class RecursiveMutex
{
    SemaphoreHandle_t mMutexHandle = nullptr;

public:
    RecursiveMutex(void);
    RecursiveMutex(const RecursiveMutex&) = delete;
    RecursiveMutex(RecursiveMutex&&);
    RecursiveMutex& operator=(const RecursiveMutex&) = delete;
    RecursiveMutex& operator=(RecursiveMutex&&);
    ~RecursiveMutex(void);

    bool take(uint32_t ticksToWait = portMAX_DELAY) const;
    bool give(void) const;

    operator bool() const;
};
}
