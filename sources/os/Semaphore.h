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
class Semaphore
{
    SemaphoreHandle_t mSemaphoreHandle = nullptr;
    bool take(uint32_t ticksToWait) const;

public:
    Semaphore(void);
    Semaphore(const Semaphore&) = delete;
    Semaphore(Semaphore&&);
    Semaphore& operator=(const Semaphore&) = delete;
    Semaphore& operator=(Semaphore&&);
    ~Semaphore(void);

    bool take(void) const {return this->take(portMAX_DELAY); }
    template<class rep, class period>
    bool take(const std::chrono::duration<rep, period>& d) const
    {
        return take(std::chrono::duration_cast<std::chrono::milliseconds>(d).count() / portTICK_RATE_MS);
    }
    bool give(void) const;
    bool giveFromISR(void) const;
    bool takeFromISR(void) const;

    operator bool() const;
};
}
