/* Copyright (C) 2016 Nils Weiss
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

#ifndef SOURCES_DEMOEXECUTER_H_
#define SOURCES_DEMOEXECUTER_H_

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

#endif /* SOURCES_DEMOEXECUTER_H_ */
