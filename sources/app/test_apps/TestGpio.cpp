// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "TestGpio.h"
#include "Gpio.h"
#include "Mpu.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

extern app::Mpu* g_Mpu;

os::TaskEndless gpioTest("Gpio_Test", 2048, os::Task::Priority::MEDIUM, [](const bool&){
                         constexpr const hal::Gpio& out = hal::Factory<hal::Gpio>::get<hal::Gpio::TEST_PIN_OUT>();
                         while (true) {
                             os::ThisTask::sleep(std::chrono::milliseconds(300));
                             out = true;
                             os::ThisTask::sleep(std::chrono::milliseconds(300));
                             out = false;
                             Eigen::Vector3f gravity = g_Mpu->getGravity();
                             Trace(ZONE_INFO, "Gravity: x:%6d, y:%6d, z:%6d\n",
                                   static_cast<int32_t>(gravity.x() * 1000),
                                   static_cast<int32_t>(gravity.y() * 1000),
                                   static_cast<int32_t>(gravity.z() * 1000));
                         }
    });
