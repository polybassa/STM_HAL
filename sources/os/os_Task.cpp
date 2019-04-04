// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "os_Task.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

#if defined (STM32F303xC) || defined (STM32F334x8) || defined (STM32F302x8) || defined (STM32F303xE)
#include "stm32f30x_it.h"
#include "stm32f30x_misc.h"
#include "system_stm32f30x.h"
#endif
#if defined (STM32F10X_LD) || defined (STM32F10X_LD_VL) || defined (STM32F10X_MD) || defined (STM32F10X_HD) || \
    defined (STM32F10X_HD_VL) || defined (STM32F10X_XL)
#include "stm32f10x_it.h"
#include "misc.h"
#include "system_stm32f10x.h"
#endif
#if defined(STM32F40_41xxx) || defined(STM32F427_437xx) || defined(STM32F429_439xx) || defined(STM32F401xx) || \
    defined(STM32F410xx) || \
    defined(STM32F411xE) || defined(STM32F412xG) || defined(STM32F413_423xx) || defined(STM32F446xx) || \
    defined(STM32F469_479xx)
#include "stm32f4xx_it.h"
#include "stm32f4xx_misc.h"
#include "system_stm32f4xx.h"
#endif

using os::Task;

bool Task::schedulerRunning = false;

Task::Task(char const* const                      name,
           const uint16_t                         stackSize,
           const os::Task::Priority               priority,
           const std::function<void(const bool&)> function) :
    mTaskFunction(function)
{
    xTaskCreate(Task::task,
                name,
                Task::STACKSIZE_IN_BYTE(stackSize),
                this,
                static_cast<uint16_t>(priority),
                &this->mHandle);
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
        const size_t countervalue = ms.count() * (SystemCoreClock / 5000);
        for (size_t i = 0; i < countervalue; i++) {
            __NOP();
        }
    }
}

/**
 * Method for fixed period scheduling.
 * @params lastWakeTickCount last time the task was unblocked, initialize this once with the current system tick.
 *             This value is upodated automatically between each iteration.
 * @params increment period duration (measured from the lastWakeTickCount)
 */
void os::ThisTask::sleepUntil(uint32_t& lastWakeTickCount, const std::chrono::milliseconds& increment)
{
    if (os::Task::isSchedulerRunning()) {
        vTaskDelayUntil(&lastWakeTickCount, ((increment.count() / portTICK_RATE_MS)));
    } else {
        const size_t countervalue = increment.count() * (SystemCoreClock / 5000);
        for (size_t i = 0; i < countervalue; i++) {
            __NOP();
        }
    }
}

void os::ThisTask::yield(void)
{
    portYIELD();
}

char* os::ThisTask::getName(void)
{
    return pcTaskGetTaskName(nullptr);
}

void os::ThisTask::enterCriticalSection(void)
{
    taskENTER_CRITICAL();
}

void os::ThisTask::exitCriticalSection(void)
{
    taskEXIT_CRITICAL();
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
    Trace(ZONE_ERROR, "MALLOC FAILED %s \r\n", pcTaskGetTaskName(nullptr));

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
    Trace(ZONE_ERROR, "STACK OVERFLOW %s\r\n", pcTaskName);

    /* Run time stack overflow checking is performed if
       configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
       function is called if a stack overflow is detected. */
    taskDISABLE_INTERRUPTS();
    for ( ; ; ) {}
}
