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

#include "TestMsCom.h"
#include "Gpio.h"
#include "virtual_Light.h"
#include "trace.h"

static const int __attribute__((used)) g_DebugZones = ZONE_ERROR | ZONE_WARNING |
                                                      ZONE_VERBOSE | ZONE_INFO;
using os::TaskEndless;

extern virt::Light* g_light1; // = nullptr;

const TaskEndless app::testMsCom("MsCom", 2048, os::Task::Priority::LOW, [] (const bool&){
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
