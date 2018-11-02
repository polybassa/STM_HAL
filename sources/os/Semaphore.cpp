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

#include "Semaphore.h"

using os::Semaphore;

Semaphore::Semaphore(void) :
    mSemaphoreHandle(xSemaphoreCreateBinary())
{}

Semaphore::Semaphore(Semaphore&& rhs) :
    mSemaphoreHandle(rhs.mSemaphoreHandle)
{
    rhs.mSemaphoreHandle = nullptr;
}

Semaphore& Semaphore::operator=(Semaphore&& rhs)
{
    mSemaphoreHandle = rhs.mSemaphoreHandle;
    rhs.mSemaphoreHandle = nullptr;
    return *this;
}

Semaphore::~Semaphore(void)
{
    vSemaphoreDelete(mSemaphoreHandle);
}

bool Semaphore::take(uint32_t ticksToWait) const
{
    return *this ? xSemaphoreTake(mSemaphoreHandle, ticksToWait) : false;
}

bool Semaphore::give(void) const
{
    return *this ? xSemaphoreGive(mSemaphoreHandle) : false;
}

bool Semaphore::giveFromISR(void) const
{
    BaseType_t highPriorityTaskWoken = 0;
    bool retVal = *this ? xSemaphoreGiveFromISR(mSemaphoreHandle, &highPriorityTaskWoken) : false;
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retVal;
}

bool Semaphore::takeFromISR(void) const
{
    BaseType_t highPriorityTaskWoken = 0;
    bool retVal = *this ? xSemaphoreTakeFromISR(mSemaphoreHandle, &highPriorityTaskWoken) : false;
    if (highPriorityTaskWoken) {
        ThisTask::yield();
    }
    return retVal;
}

Semaphore::operator bool() const
{
    return mSemaphoreHandle != nullptr;
}
