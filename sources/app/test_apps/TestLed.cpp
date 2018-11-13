// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "TestLed.h"
#include "trace.h"
#include "Light.h"
#include <chrono>

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

os::TaskEndless ledTest("LED_Test", 1024, os::Task::Priority::MEDIUM, [](const bool&){
                        const dev::Light& headLight = dev::Factory<dev::Light>::get<interface ::Light::HEADLIGHT>();
                        const dev::Light& backLight = dev::Factory<dev::Light>::get<interface ::Light::BACKLIGHT>();

                        while (true) {
                            os::ThisTask::sleep(std::chrono::seconds(2));
                            backLight.setColor({0, 0, 255});
                            os::ThisTask::sleep(std::chrono::seconds(2));
                            backLight.displayNumber(0, {0, 255, 0});
                            os::ThisTask::sleep(std::chrono::seconds(2));
                            backLight.displayNumber(8, {0, 255, 0});
                            os::ThisTask::sleep(std::chrono::seconds(2));
                            backLight.displayNumber(1, {0, 255, 0});
                            os::ThisTask::sleep(std::chrono::seconds(2));
                            backLight.displayNumber(5, {0, 255, 0});
                            os::ThisTask::sleep(std::chrono::seconds(2));
                            backLight.displayNumber(9, {0, 255, 0});
                            os::ThisTask::sleep(std::chrono::seconds(2));
                            backLight.displayNumber(3, {0, 255, 0});
                            os::ThisTask::sleep(std::chrono::seconds(2));
                            backLight.setColor({0, 0, 255});
                            os::ThisTask::sleep(std::chrono::seconds(2));
                        }
    });
