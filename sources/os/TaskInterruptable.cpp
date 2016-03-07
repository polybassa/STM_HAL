/* Copyright (C) 2015  Nils Weiss
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "TaskInterruptable.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using os::TaskInterruptable;

TaskInterruptable::TaskInterruptable(const char* name, uint16_t stackSize, os::Task::Priority priority,
                                     std::function<void(const bool&)> function) :
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
    this->mJoinFlag = true;
    xSemaphoreTake(this->mJoinSemaphore, portMAX_DELAY);
    Trace(ZONE_INFO, "%s joined \r\n", this->getName());
}

void TaskInterruptable::detach(void)
{
    this->mJoinFlag = true;
    Trace(ZONE_INFO, "%s detached \r\n", this->getName());
}
