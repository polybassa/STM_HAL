/// @file TaskInterruptableTestMockup.cpp
/// @brief TODO brief description.
/// @author Henning Mende (henning@my-urmo.com)
/// @date   Nov 26, 2019
/// @copyright UrmO GmbH
///
/// Unauthorized copying of this file, via any medium is strictly prohibited.
/// Proprietary and confidential.
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
