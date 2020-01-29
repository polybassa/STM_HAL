/// @file MutexTestMockup.cpp
/// @brief Mockup for software tests of classes dependent on os::Mutex.
/// @author Henning Mende (henning@my-urmo.com)
/// @date   Nov 19, 2019
/// @copyright UrmO GmbH
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
/// If not, see <https://www.gnu.org/licenses/>.
#include "Mutex.h"
#include <mutex>

namespace os
{
Mutex::Mutex(void) :
    mMutexHandle((SemaphoreHandle_t) new std::timed_mutex)
{}

Mutex::Mutex(Mutex&& rhs) :
    mMutexHandle(rhs.mMutexHandle)
{
    rhs.mMutexHandle = nullptr;
}

Mutex& Mutex::operator=(Mutex&& rhs)
{
    mMutexHandle = rhs.mMutexHandle;
    rhs.mMutexHandle = nullptr;
    return *this;
}

Mutex::~Mutex(void)
{
    delete (int*)mMutexHandle;
    mMutexHandle = nullptr;
}

bool Mutex::take(uint32_t ticksToWait) const
{
    if (*this) {
        std::timed_mutex* m = reinterpret_cast<std::timed_mutex*>(mMutexHandle);
        return m->try_lock_for(std::chrono::milliseconds(ticksToWait));
    }

    return false;
}

bool Mutex::give(void) const
{
    if (*this) {
        std::timed_mutex* m = reinterpret_cast<std::timed_mutex*>(mMutexHandle);
        m->unlock();
        return true;
    }
    return false;
}

Mutex::operator bool() const
{
    return mMutexHandle != nullptr;
}
}
