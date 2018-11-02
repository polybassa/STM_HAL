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

#include "CountingSemaphore.h"

using os::CountingSemaphore;

CountingSemaphore::CountingSemaphore(uint32_t maximalCount,
                                     uint32_t initalCount) :
    mSemaphoreHandle(xSemaphoreCreateCounting(
                                              maximalCount,
                                              initalCount))
{}

CountingSemaphore::CountingSemaphore(CountingSemaphore&& rhs) :
    mSemaphoreHandle(rhs.mSemaphoreHandle)
{
    rhs.mSemaphoreHandle = nullptr;
}

CountingSemaphore& CountingSemaphore::operator=(CountingSemaphore&& rhs)
{
    mSemaphoreHandle = rhs.mSemaphoreHandle;
    rhs.mSemaphoreHandle = nullptr;
    return *this;
}

CountingSemaphore::~CountingSemaphore(void)
{
    vSemaphoreDelete(mSemaphoreHandle);
}

bool CountingSemaphore::take(uint32_t ticksToWait) const
{
    return *this ? xSemaphoreTake(mSemaphoreHandle, ticksToWait) : false;
}

bool CountingSemaphore::give(void) const
{
    return *this ? xSemaphoreGive(mSemaphoreHandle) : false;
}

bool CountingSemaphore::giveFromISR(void) const
{
    BaseType_t highPriorityTaskWoken = 0;
    bool retVal = *this ? xSemaphoreGiveFromISR(mSemaphoreHandle, &highPriorityTaskWoken) : false;
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retVal;
}

bool CountingSemaphore::takeFromISR(void) const
{
    BaseType_t highPriorityTaskWoken = 0;
    bool retVal = *this ? xSemaphoreTakeFromISR(mSemaphoreHandle, &highPriorityTaskWoken) : false;
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retVal;
}

CountingSemaphore::operator bool() const
{
    return mSemaphoreHandle != nullptr;
}
