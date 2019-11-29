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
    mSemaphoreHandle((SemaphoreHandle_t) new std::mutex)
{
    std::mutex* m = reinterpret_cast<std::mutex*>(mSemaphoreHandle);
    m->lock();
}

Semaphore::Semaphore(Semaphore&& rhs) :
    mSemaphoreHandle(rhs.mSemaphoreHandle)
{
    rhs.mSemaphoreHandle = nullptr;
    std::mutex* m = reinterpret_cast<std::mutex*>(mSemaphoreHandle);
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
        std::mutex* m = reinterpret_cast<std::mutex*>(mSemaphoreHandle);

        bool noUnclockByAwakener = true;
        std::thread awakener([&] {
                             if (ticksToWait != portMAX_DELAY) {
                                 std::mutex* pm = reinterpret_cast<std::mutex*>(mSemaphoreHandle);
                                 std::this_thread::sleep_for(std::chrono::milliseconds(ticksToWait));
                                 if (!pm->try_lock()) {
                                     noUnclockByAwakener = false;
                                     pm->unlock();
                                 }
                             }
                });

        m->lock();

        awakener.detach();

        return noUnclockByAwakener;
    }

    return false;
}

bool Semaphore::give(void) const
{
    if (*this) {
        std::mutex* m = reinterpret_cast<std::mutex*>(mSemaphoreHandle);
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
