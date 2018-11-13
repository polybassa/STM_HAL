// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "FreeRTOS.h"
#include "semphr.h"
#include "os_Task.h"

namespace os
{
class CountingSemaphore
{
    SemaphoreHandle_t mSemaphoreHandle = nullptr;

public:
    CountingSemaphore(uint32_t maximalCount, uint32_t initalCount);
    CountingSemaphore(const CountingSemaphore&) = delete;
    CountingSemaphore(CountingSemaphore&&);
    CountingSemaphore& operator=(const CountingSemaphore&) = delete;
    CountingSemaphore& operator=(CountingSemaphore&&);
    ~CountingSemaphore(void);

    bool take(uint32_t ticksToWait = portMAX_DELAY) const;
    bool give(void) const;
    bool giveFromISR(void) const;
    bool takeFromISR(void) const;

    operator bool() const;
};
}
