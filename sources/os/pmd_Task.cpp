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

#include "pmd_Task.h"
#include "stm32f30x_it.h"
#include "stm32f30x_misc.h"

using os::Task;

bool Task::schedulerRunning = false;

Task::Task(const char* name, uint16_t stackSize, uint32_t priority,
           std::function<void(const bool&)> function) : mTaskFunction(
        function)
{
    xTaskCreate(Task::task, name, Task::STACKSIZE_IN_BYTE(stackSize), this, priority, &this->mHandle);
}

Task::~Task(void)
{
    if (this->mHandle) {
        vTaskDelete(this->mHandle);
    }
}

void Task::task(void* pvParameters)
{
    static_cast<Task*>(pvParameters)->taskFunction();
}

void Task::taskFunction(void)
{
    mTaskFunction(false);
    vTaskDelete(nullptr);
    this->mHandle = 0;
}

void Task::suspend(void) const
{
    vTaskSuspend(this->mHandle);
}

void Task::resume(void) const
{
    vTaskResume(this->mHandle);
}

void Task::resumeFromISR(void) const
{
    xTaskResumeFromISR(this->mHandle);
}

uint32_t Task::getPriority(void) const
{
    return uxTaskPriorityGet(this->mHandle);
}

uint32_t Task::getPriorityFromISR(void) const
{
    return uxTaskPriorityGetFromISR(this->mHandle);
}

void Task::setPriority(const uint32_t prio) const
{
    vTaskPrioritySet(this->mHandle, prio);
}

char* Task::getName(void) const
{
    return pcTaskGetTaskName(this->mHandle);
}

void Task::startScheduler(void)
{
#ifndef UNITTEST
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
#endif
    schedulerRunning = true;
    vTaskStartScheduler();
}

void Task::endScheduler(void)
{
    vTaskEndScheduler();
    schedulerRunning = false;
}

bool Task::isSchedulerRunning(void)
{
    return schedulerRunning;
}

void Task::suspendAll(void)
{
    vTaskSuspendAll();
}

void Task::resumeAll(void)
{
    xTaskResumeAll();
}

uint32_t Task::getNumberOfTasks(void)
{
    return uxTaskGetNumberOfTasks();
}

uint32_t Task::getTickCount(void)
{
    return xTaskGetTickCount();
}

uint32_t Task::getTickCountFromISR(void)
{
    return xTaskGetTickCountFromISR();
}

void os::ThisTask::sleep(const std::chrono::milliseconds ms)
{
    if (os::Task::isSchedulerRunning()) {
        vTaskDelay(ms.count() / portTICK_RATE_MS);
    } else {
        for (size_t i = 0; i < ms.count() * 1200; i++) {
            __NOP();
        }
    }
}

void os::ThisTask::yield(void)
{
    portYIELD();
}

extern "C" void vApplicationTickHook(void) {}

/*-----------------------------------------------------------*/
extern "C" void vApplicationMallocFailedHook(void)
{
    /* vApplicationMallocFailedHook() will only be called if
       configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
       function that will get called if a call to pvPortMalloc() fails.
       pvPortMalloc() is called internally by the kernel whenever a task, queue,
       timer or semaphore is created.  It is also called by various parts of the
       demo application.  If heap_1.c or heap_2.c are used, then the size of the
       heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
       FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
       to query the size of free heap space that remains (although it does not
       provide information on how the remaining heap might be fragmented). */
    taskDISABLE_INTERRUPTS();
    for ( ; ; ) {}
}

/*-----------------------------------------------------------*/
extern "C" void vApplicationIdleHook(void)
{
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
       to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
       task.  It is essential that code added to this hook function never attempts
       to block in any way (for example, call xQueueReceive() with a block time
       specified, or call vTaskDelay()).  If the application makes use of the
       vTaskDelete() API function (as this demo application does) then it is also
       important that vApplicationIdleHook() is permitted to return to its calling
       function, because it is the responsibility of the idle task to clean up
       memory allocated by the kernel to any task that has since been deleted. */
}

/*-----------------------------------------------------------*/
extern "C" void vApplicationStackOverflowHook(xTaskHandle pxTask, signed char* pcTaskName)
{
    (void)pcTaskName;
    (void)pxTask;

    /* Run time stack overflow checking is performed if
       configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
       function is called if a stack overflow is detected. */
    taskDISABLE_INTERRUPTS();
    for ( ; ; ) {}
}
