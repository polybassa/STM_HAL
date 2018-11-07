// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "FreeRTOS.h"
#include "semphr.h"

namespace os
{
class Mutex
{
    SemaphoreHandle_t mMutexHandle = nullptr;

public:
    Mutex(void);
    Mutex(const Mutex&) = delete;
    Mutex(Mutex&&);
    Mutex& operator=(const Mutex&) = delete;
    Mutex& operator=(Mutex&&);
    ~Mutex(void);

    bool take(uint32_t ticksToWait = portMAX_DELAY) const;
    bool give(void) const;

    operator bool() const;
};
}
