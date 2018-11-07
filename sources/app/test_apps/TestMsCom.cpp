// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "TestMsCom.h"
#include "Gpio.h"
#include "virtual_Light.h"
#include "trace.h"

static const int __attribute__((used)) g_DebugZones = ZONE_ERROR | ZONE_WARNING |
                                                      ZONE_VERBOSE | ZONE_INFO;
using os::TaskEndless;

extern virt::Light* g_light1; // = nullptr;

const TaskEndless app::testMsCom("MsCom", 2048, os::Task::Priority::LOW, [](const bool&){
                                 constexpr auto& master = hal::Factory<hal::Gpio>::get<hal::Gpio::CONFIG>();

                                 Trace(ZONE_INFO, "Start MS Com test");

                                 uint8_t value = 0;

                                 while (true) {
                                     if (!master) {
                                         auto color = g_light1->getColor();
                                         Trace(ZONE_INFO, "R:%d G:%d B:%d\r\n", color.red, color.green, color.blue);

                                         os::ThisTask::sleep(std::chrono::milliseconds(50));
                                     } else {
                                         g_light1->setColor({value, static_cast<uint8_t>(255 - value), value});

                                         os::ThisTask::sleep(std::chrono::milliseconds(50));
                                     }
                                 }
    });
