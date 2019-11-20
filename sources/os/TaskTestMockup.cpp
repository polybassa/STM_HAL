/// @file TaskTestMockup.cpp
/// @brief Alternate implementation for os::Task, to map tasks to c++ std::threads.
/// @author Henning Mende (henning@my-urmo.com)
/// @date   Nov 19, 2019
/// @copyright UrmO GmbH
///
/// This implementation is intended for software tests of classes that depend on os::Thread.
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

#include "os_Task.h"
#include "thread"

os::Task::Task(char const* name, unsigned short stack, os::Task::Priority prio,
               std::function<void(bool const&)> func) : mHandle(nullptr), mTaskFunction(func)
{
    mHandle = reinterpret_cast<xTaskHandle>(new std::thread([this] {
        taskFunction();
    }));
}

os::Task::~Task(void)
{
    if (mHandle != nullptr) {
        std::thread* pTmp = reinterpret_cast<std::thread*>(mHandle);
        pTmp->join();
        delete pTmp;
    }
}

void os::Task::taskFunction(void)
{
    mTaskFunction(false);
}

void os::ThisTask::sleep(const std::chrono::milliseconds ms)
{
    std::this_thread::sleep_for(ms);
}

void os::ThisTask::yield(void)
{
    std::this_thread::yield();
}
