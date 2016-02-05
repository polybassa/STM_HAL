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

#ifndef SOURCES_PMD_TASK_INTERRUPTABLE_H_
#define SOURCES_PMD_TASK_INTERRUPTABLE_H_

#include "os_Task.h"
#include "semphr.h"

namespace os
{
class TaskInterruptable : public Task {
    xSemaphoreHandle mJoinSemaphore;
    bool mJoinFlag;

public:
    TaskInterruptable(const char* name, uint16_t stackSize, uint32_t priority,
                      std::function<void(const bool&)> function);
    virtual ~TaskInterruptable(void) override;
    using Task::Task;

    virtual void taskFunction(void) override;
    void start(void);
    void join(void);
    void detach(void);
};
}

#endif /* SOURCES_PMD_TASK_INTERRUPTABLE_H_ */
