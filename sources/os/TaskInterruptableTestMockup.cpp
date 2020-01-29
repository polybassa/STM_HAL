/// @file TaskInterruptableTestMockup.cpp
/// @brief      Mockup for software tests of classes dependent on os::TaskInterruptable.
/// @author Henning Mende (henning@my-urmo.com)
/// @date   Nov 26, 2019
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
#include "TaskInterruptable.h"
#include "Semaphore.h"
#include <thread>

/// @brief Set this true in tests to execute the tasks.
/// Otherwise the os::Task interface is linked to empty function bodies.
extern bool executeMockupTasks;

namespace os
{
TaskInterruptable::TaskInterruptable(char const* const                      name,
                                     const uint16_t                         stackSize,
                                     const os::Task::Priority               priority,
                                     const std::function<void(const bool&)> function) :
    Task(name, stackSize, priority, function),
    mJoinFlag(false)
{
    if (executeMockupTasks) {
        mJoinSemaphore = reinterpret_cast<xSemaphoreHandle>(new os::Semaphore());
    }
}

TaskInterruptable::~TaskInterruptable(void)
{
    if ((mHandle != nullptr) && executeMockupTasks) {
        join();

        os::Semaphore* pSemaphore = reinterpret_cast<os::Semaphore*>(mJoinSemaphore);
        delete pSemaphore;
    }
}

void TaskInterruptable::taskFunction(void)
{
    mTaskFunction(mJoinFlag);
    std::thread* pTmp = reinterpret_cast<std::thread*>(mHandle);
    mHandle = nullptr;
    pTmp->detach();
    delete pTmp;
    reinterpret_cast<os::Semaphore*>(mJoinSemaphore)->give();
}

void TaskInterruptable::start(void)
{
    mJoinFlag = false;
    if ((mHandle == nullptr) && executeMockupTasks) {
        mHandle = reinterpret_cast<xTaskHandle>(new std::thread([this] {
                taskFunction();
            }));
    }
}

void TaskInterruptable::join(void)
{
    mJoinFlag = true;
    if ((mHandle != nullptr) && executeMockupTasks) {
        reinterpret_cast<os::Semaphore*>(mJoinSemaphore)->take();
    }
}

void TaskInterruptable::detach(void)
{
    this->mJoinFlag = true;
}
}  // namespace os
