// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "DemoExecuter.h"
#include "trace.h"
#include <cstdlib>

using app::CanController;
using app::DemoExecuter;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

DemoExecuter::DemoExecuter(CanController& can) :
    os::DeepSleepModule(), mDemoExecuterTask("DemoExecuter",
                                             DemoExecuter::STACKSIZE, os::Task::Priority::LOW,
                                             [this](const bool& join)
{
    DemoExecuterTaskFunction(join);
}), mCan(can), mDemoQueue()
{}

void DemoExecuter::enterDeepSleep(void)
{
    mDemoExecuterTask.join();
}

void DemoExecuter::exitDeepSleep(void)
{
    mDemoExecuterTask.start();
}

void DemoExecuter::send_GM_tester_present_twice()
{
    mCan.send("t2412013E\r");
    os::ThisTask::sleep(std::chrono::milliseconds(50));
    mCan.send("t2412013E\r");
    os::ThisTask::sleep(std::chrono::milliseconds(50));
}

void DemoExecuter::demo_wipers_run(const char* args)
{
    Trace(ZONE_INFO, "Running demo Wipers!\r\n");
    send_GM_tester_present_twice();
    mCan.send("t241807AE038000030000\r");
}

void DemoExecuter::demo_horn_run(const char* args)
{
    Trace(ZONE_INFO, "Running demo Honk!\r\n");
    send_GM_tester_present_twice();
    mCan.send("t241807AE100101000000\r");
    os::ThisTask::sleep(std::chrono::milliseconds(100));
    mCan.send("t241807AE100100000000\r");
}

void DemoExecuter::demo_doors_run(const char* args)
{
    Trace(ZONE_INFO, "Running demo Doors!\r\n");
    send_GM_tester_present_twice();
    mCan.send("t241807AE010404000000\r");
    os::ThisTask::sleep(std::chrono::milliseconds(1000));
    send_GM_tester_present_twice();
    mCan.send("t241807AE010202000000\r");
}

void DemoExecuter::demo_window_run(const char* args)
{
    Trace(ZONE_INFO, "Running demo Window!\r\n");
    send_GM_tester_present_twice();
    mCan.send("t241807AE3B0101000000\r");
    os::ThisTask::sleep(std::chrono::milliseconds(3000));
    send_GM_tester_present_twice();
    mCan.send("t241807AE3B0102000000\r");
}

void DemoExecuter::demo_lights_run(const char* args)
{
    Trace(ZONE_INFO, "Running demo Lights!\r\n");
    send_GM_tester_present_twice();
    os::ThisTask::sleep(std::chrono::milliseconds(1000));
    send_GM_tester_present_twice();
    mCan.send("t241807AE02f0f0787800\r"); // Front Fog Lamps
    os::ThisTask::sleep(std::chrono::milliseconds(1000));
    send_GM_tester_present_twice();
    mCan.send("t241807AE070380008000\r"); // Left Park Lamps
    os::ThisTask::sleep(std::chrono::milliseconds(1000));
    send_GM_tester_present_twice();
    mCan.send("t241807AE0F0404000000\r"); // License Plate Lamps
    os::ThisTask::sleep(std::chrono::milliseconds(1000));
    send_GM_tester_present_twice();
    mCan.send("t241807AE1A0380008000\r"); // Right Stop Lamp
    os::ThisTask::sleep(std::chrono::milliseconds(1000));
    send_GM_tester_present_twice();
    mCan.send("t241504AE730303\r");       // Headlamp Low Beam
    os::ThisTask::sleep(std::chrono::milliseconds(1000));
    send_GM_tester_present_twice();
    mCan.send("t241504AE740303\r");       // Dedicated Daytime Running Lamp
    /* // These lights groups are activated using bitmasks
       os::ThisTask::sleep(std::chrono::milliseconds(1000));send_GM_tester_present_twice();mCan.send("t241807AE020000080800\r"); // Backup Lamps
       os::ThisTask::sleep(std::chrono::milliseconds(1000));send_GM_tester_present_twice();mCan.send("t241807AE020000101000\r"); // Rear Fog Lamp(s) Relay
       os::ThisTask::sleep(std::chrono::milliseconds(1000));send_GM_tester_present_twice();mCan.send("t241807AE020000202000\r"); // Center Stop Lamp
       os::ThisTask::sleep(std::chrono::milliseconds(1000));send_GM_tester_present_twice();mCan.send("t241807AE020000404000\r"); // Front Fog Lamps
       os::ThisTask::sleep(std::chrono::milliseconds(1000));send_GM_tester_present_twice();mCan.send("t241807AE021010000000\r"); // Left Front Turn Signal Lamp
       os::ThisTask::sleep(std::chrono::milliseconds(1000));send_GM_tester_present_twice();mCan.send("t241807AE022020000000\r"); // Left Rear Turn Signal Lamp
       os::ThisTask::sleep(std::chrono::milliseconds(1000));send_GM_tester_present_twice();mCan.send("t241807AE024040000000\r"); // Right Front Turn Signal Lamp
       os::ThisTask::sleep(std::chrono::milliseconds(1000));send_GM_tester_present_twice();mCan.send("t241807AE028080000000\r"); // Right Rear Turn Signal Lamp

       os::ThisTask::sleep(std::chrono::milliseconds(1000));send_GM_tester_present_twice();mCan.send("t241807AE070180000000\r"); // Left Park Lamps
       os::ThisTask::sleep(std::chrono::milliseconds(1000));send_GM_tester_present_twice();mCan.send("t241807AE070200008000\r"); // Right Park Lamps

       os::ThisTask::sleep(std::chrono::milliseconds(1000));send_GM_tester_present_twice();mCan.send("t241807AE0F0404000000\r"); // License Plate Lamps
       os::ThisTask::sleep(std::chrono::milliseconds(1000));send_GM_tester_present_twice();mCan.send("t241807AE1A0180000000\r"); // Right Stop Lamp
       os::ThisTask::sleep(std::chrono::milliseconds(1000));send_GM_tester_present_twice();mCan.send("t241807AE1A0200008000\r"); // Left Stop Lamp

       os::ThisTask::sleep(std::chrono::milliseconds(1000));send_GM_tester_present_twice();mCan.send("t241504AE730101\r");       // Left Headlamp Low Beam
       os::ThisTask::sleep(std::chrono::milliseconds(1000));send_GM_tester_present_twice();mCan.send("t241504AE730202\r");       // Right Headlamp Low Beam
       os::ThisTask::sleep(std::chrono::milliseconds(1000));send_GM_tester_present_twice();mCan.send("t241504AE740101\r");       // Left Dedicated Daytime Running Lamp
       os::ThisTask::sleep(std::chrono::milliseconds(1000));send_GM_tester_present_twice();mCan.send("t241504AE740202\r");       // Right Dedicated Daytime Running Lamp
     */

    os::ThisTask::sleep(std::chrono::milliseconds(5000));

    send_GM_tester_present_twice();
    mCan.send("t241807AE00\r"); // Release Control
}

void DemoExecuter::demo_washers_run(const char* args)
{
    Trace(ZONE_INFO, "Running demo Washers!\r\n");
    send_GM_tester_present_twice();
    mCan.send("t241807AE030808000000\r");
    os::ThisTask::sleep(std::chrono::milliseconds(1000));
    send_GM_tester_present_twice();
    mCan.send("t241807AE030800000000\r");
    mCan.send("t241807AE00\r"); // Release Control
}

void DemoExecuter::DemoExecuterTaskFunction(const bool& join)
{
    os::ThisTask::sleep(std::chrono::seconds(5));

    Trace(ZONE_INFO, "Start DemoExecuter\r\n");
    std::array<char, 10> demoBuffer;

    do {
        demoBuffer.fill(0);
        auto received = mDemoQueue.receive(demoBuffer);

        if (received) {
            int demo_index = std::strtol(demoBuffer.data(), NULL, 10);
            Trace(ZONE_INFO, "Demo %d requested.\r\n", demo_index);

            switch (demo_index) {
            case 0:
                demo_wipers_run(nullptr);
                break;

            case 1:
                demo_horn_run(nullptr);
                break;

            case 2:
                demo_doors_run(nullptr);
                break;

            case 3:
                demo_window_run(nullptr);
                break;

            case 4:
                demo_lights_run(nullptr);
                break;

            case 5:
                demo_washers_run(nullptr);
                break;
            }
        }
    } while (!join);
}

void DemoExecuter::runDemo(std::string_view data)
{
    std::array<char, 10> tempData;
    data.copy(tempData.data(), std::min(tempData.size(), data.length()));
    mDemoQueue.sendFront(tempData, std::chrono::milliseconds(0));
}
