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

#include "TestGpio.h"
#include "Gpio.h"

#include "RealTimeDebugInterface.h"
#include "Mpu.h"

extern dev::RealTimeDebugInterface* g_RTTerminal;
extern app::Mpu* g_Mpu;

os::TaskEndless gpioTest("Gpio_Test", 2048, os::Task::Priority::MEDIUM, [] (const bool&){
                             constexpr const hal::Gpio& out = hal::Factory<hal::Gpio>::get<hal::Gpio::TEST_PIN_OUT>();
                             while (true) {
                                 os::ThisTask::sleep(std::chrono::milliseconds(300));
                                 out = true;
                                 os::ThisTask::sleep(std::chrono::milliseconds(300));
                                 out = false;
                                 Eigen::Vector3f gravity = g_Mpu->getGravity();
                                 g_RTTerminal->printf("Gravity: x:%6d, y:%6d, z:%6d\n",
                                                      static_cast<int32_t>(gravity.x() * 1000),
                                                      static_cast<int32_t>(gravity.y() * 1000),
                                                      static_cast<int32_t>(gravity.z() * 1000));
                             }
                         });
