// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "CanController.h"
#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "os_Queue.h"
#include <array>
#include <string_view>

namespace app
{
class DemoExecuter final :
    private os::DeepSleepModule
{
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr uint32_t STACKSIZE = 1024;

    os::TaskInterruptable mDemoExecuterTask;
    CanController& mCan;
    os::Queue<std::array<char, 10>, 10> mDemoQueue;

    const std::chrono::milliseconds mExecutionInterval = std::chrono::milliseconds(100);

    void DemoExecuterTaskFunction(const bool&);

    void send_GM_tester_present_twice(void);
    void demo_wipers_run(const char* args);
    void demo_horn_run(const char* args);
    void demo_doors_run(const char* args);
    void demo_window_run(const char* args);
    void demo_lights_run(const char* args);
    void demo_washers_run(const char* args);

public:
    DemoExecuter(CanController& can);

    DemoExecuter(const DemoExecuter&) = delete;
    DemoExecuter(DemoExecuter&&) = delete;
    DemoExecuter& operator=(const DemoExecuter&) = delete;
    DemoExecuter& operator=(DemoExecuter&&) = delete;

    void runDemo(std::string_view data);
};
}
