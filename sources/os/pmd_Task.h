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

#ifndef SOURCES_PMD__TASK_H_
#define SOURCES_PMD__TASK_H_

#include <cstdint>
#include "FreeRTOS.h"
#include "task.h"
#include <functional>
#include <chrono>

namespace os
{
class Task {
    static void task(void* pvParameters);
    static bool schedulerRunning;

protected:
    virtual void taskFunction(void);

    xTaskHandle mHandle;
    std::function<void(const bool&)> mTaskFunction;

public:
    enum Priority {
        HIGHEST = 15,
        VERY_HIGH = 13,
        HIGH = 10,
        MEDIUM = 7,
        LOW = 4,
        VERY_LOW = 2,
        LOWEST = 1
    };

    Task(const char* name, uint16_t stackSize, uint32_t priority,
         std::function<void(const bool&)> function);
    Task(const Task&) = delete;
    Task(Task&&) = delete;
    Task& operator=(const Task&) = delete;
    Task& operator=(Task&&) = delete;

    virtual ~Task(void);

    void suspend(void) const;
    void resume(void) const;
    void resumeFromISR(void) const;

    uint32_t getPriority(void) const;
    uint32_t getPriorityFromISR(void) const;
    void setPriority(const uint32_t) const;

    static bool isSchedulerRunning(void);

    char* getName(void) const;

    static constexpr size_t STACKSIZE_IN_BYTE(size_t n)
    {
        return n / (sizeof(portSTACK_TYPE));
    }

    static void startScheduler(void);
    static void endScheduler(void);
    static void suspendAll(void);
    static void resumeAll(void);
    static uint32_t getNumberOfTasks(void);
    static uint32_t getTickCount(void);
    static uint32_t getTickCountFromISR(void);

    friend struct ThisTask;
};

struct ThisTask {
    template<class rep, class period>
    static void sleep(const std::chrono::duration<rep, period>& d)
    {
        sleep(std::chrono::duration_cast<std::chrono::milliseconds>(d));
    }

    static void sleep(const std::chrono::milliseconds ms);
    static void yield(void);
};
}

#endif /* SOURCES_PMD__TASK_H_ */
