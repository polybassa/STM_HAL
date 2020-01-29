/// @file SemaphoreTestMockup.cpp
/// @brief Mockup for software tests of classes dependent on os::Semaphore.
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
#include "Semaphore.h"

#include <mutex>
#include <thread>

namespace os
{
Semaphore::Semaphore(void) :
    mSemaphoreHandle((SemaphoreHandle_t) new std::timed_mutex)
{
    std::timed_mutex* m = reinterpret_cast<std::timed_mutex*>(mSemaphoreHandle);
    m->lock();
}

Semaphore::Semaphore(Semaphore&& rhs) :
    mSemaphoreHandle(rhs.mSemaphoreHandle)
{
    rhs.mSemaphoreHandle = nullptr;
    std::timed_mutex* m = reinterpret_cast<std::timed_mutex*>(mSemaphoreHandle);
    m->lock();
}

Semaphore& Semaphore::operator=(Semaphore&& rhs)
{
    mSemaphoreHandle = rhs.mSemaphoreHandle;
    rhs.mSemaphoreHandle = nullptr;
    return *this;
}

Semaphore::~Semaphore(void)
{
    delete (int*)mSemaphoreHandle;
    mSemaphoreHandle = nullptr;
}

bool Semaphore::take(uint32_t ticksToWait) const
{
    if (*this) {
        std::timed_mutex* m = reinterpret_cast<std::timed_mutex*>(mSemaphoreHandle);
        return m->try_lock_for(std::chrono::milliseconds(ticksToWait));
    }

    return false;
}

bool Semaphore::give(void) const
{
    if (*this) {
        std::timed_mutex* m = reinterpret_cast<std::timed_mutex*>(mSemaphoreHandle);
        m->unlock();
        return true;
    }
    return false;
}

Semaphore::operator bool() const
{
    return mSemaphoreHandle != nullptr;
}
}
