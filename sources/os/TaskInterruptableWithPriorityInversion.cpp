/// @file TaskInterruptableWithPriorityInversion.cpp
/// @brief Implementation of the TaskInterruptable class with priority inversion.
/// @author Henning Mende (henning@my-urmo.com)
/// @date   Nov 19, 2019
/// @copyright UrmO GmbH
///
/// This is an alternate implementation of the TaskInterruptable class, which is based on
/// FreeRTOS Mutex instead of Semaphore. Thus a low priority task which is interrupted by
/// a Task with a high priority inherits the high priority until it is done. This mechanism
/// prevents a long time blocking of the high priority task. Additionally the xTaskAbortDelay()
/// function is used to wake the interrupted task immediately. This reduces the blocking time
/// of the interrupting task, too.
///
/// This program is free software: you can redistribute it and/or modify it under the terms
/// of the GNU General Public License as published by the Free Software Foundation, either
/// version 3 of the License, or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
/// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
/// See the GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License along with this program.

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
    mJoinSemaphore(xSemaphoreCreateMutex()),
    mJoinFlag(false)
{}

TaskInterruptable::~TaskInterruptable(void)
{
    this->Task::~Task();
    vSemaphoreDelete(this->mJoinSemaphore);
}

void TaskInterruptable::taskFunction(void)
{
    while (true) {
        xSemaphoreTake(this->mJoinSemaphore, portMAX_DELAY);
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
        xTaskAbortDelay(this->mHandle);
        xSemaphoreTake(this->mJoinSemaphore, portMAX_DELAY);
        Trace(ZONE_INFO, "%s joined \r\n", this->getName());
        xSemaphoreGive(this->mJoinSemaphore);
    } else {
        Trace(ZONE_INFO, "%s try join, but already suspended\r\n", this->getName());
    }
}

void TaskInterruptable::detach(void)
{
    this->mJoinFlag = true;
    Trace(ZONE_INFO, "%s detached \r\n", this->getName());
}
