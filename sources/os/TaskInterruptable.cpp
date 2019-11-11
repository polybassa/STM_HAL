// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "TaskInterruptable.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using os::TaskInterruptable;

TaskInterruptable::TaskInterruptable(char const* const                      name,
                                     const uint16_t                         stackSize,
                                     const os::Task::Priority               priority,
                                     const std::function<void(const bool&)> function) :
    Task(name, stackSize, priority,
         function),
    mJoinFlag(false)
{
    vSemaphoreCreateBinary(this->mJoinSemaphore);
    xSemaphoreTake(this->mJoinSemaphore, 0);
}

TaskInterruptable::~TaskInterruptable(void)
{
    this->Task::~Task();
    vSemaphoreDelete(this->mJoinSemaphore);
}

void TaskInterruptable::taskFunction(void)
{
    while (true) {
        mTaskFunction(mJoinFlag);
        xSemaphoreGive(this->mJoinSemaphore);
        this->suspend();
    }
}

void TaskInterruptable::start(void)
{
    this->mJoinFlag = false;
    Trace(ZONE_INFO, "%s started \r\n", this->getName());
    this->resume();
}

void TaskInterruptable::join(void)
{
    if (mJoinFlag == false) {
        this->mJoinFlag = true;
        xSemaphoreTake(this->mJoinSemaphore, portMAX_DELAY);
        Trace(ZONE_INFO, "%s joined \r\n", this->getName());
    } else {
        Trace(ZONE_INFO, "%s try join, but already suspended\r\n", this->getName());
    }
}

void TaskInterruptable::detach(void)
{
    this->mJoinFlag = true;
    Trace(ZONE_INFO, "%s detached \r\n", this->getName());
}
