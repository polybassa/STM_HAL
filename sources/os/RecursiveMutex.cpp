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

#include "RecursiveMutex.h"

using os::RecursiveMutex;

RecursiveMutex::RecursiveMutex(void) : mMutexHandle(xSemaphoreCreateRecursiveMutex())
{}

RecursiveMutex::RecursiveMutex(RecursiveMutex && rhs) : mMutexHandle(rhs.mMutexHandle)
{
    rhs.mMutexHandle = nullptr;
}

RecursiveMutex& RecursiveMutex::operator=(RecursiveMutex && rhs)
{
    mMutexHandle = rhs.mMutexHandle;
    rhs.mMutexHandle = nullptr;
    return *this;
}

RecursiveMutex::~RecursiveMutex(void)
{
    vSemaphoreDelete(mMutexHandle);
}

bool RecursiveMutex::take(uint32_t ticksToWait) const
{
    return *this ? xSemaphoreTakeRecursive(mMutexHandle, ticksToWait) : false;
}

bool RecursiveMutex::give(void) const
{
    return *this ? xSemaphoreGiveRecursive(mMutexHandle) : false;
}

RecursiveMutex::operator bool() const
{
    return mMutexHandle != nullptr;
}
