// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "os_Task.h"
#include "semphr.h"

namespace os
{
class TaskInterruptable :
    public Task
{
    xSemaphoreHandle mJoinSemaphore;
    bool mJoinFlag;

public:
    TaskInterruptable(char const* const name, const uint16_t stackSize, const os::Task::Priority priority,
                      const std::function<void(const bool&)> function);
    virtual ~TaskInterruptable(void) override;
    using Task::Task;

    virtual void taskFunction(void) override;
    void start(void);
    void join(void);
    void detach(void);
};
}
