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

#include "TestLed.h"
#include "trace.h"
#include "Light.h"
#include <chrono>

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

os::TaskEndless ledTest("LED_Test", 1024, os::Task::Priority::MEDIUM, [] (const bool&){
                            const dev::Light& headLight = dev::Factory<dev::Light>::get<dev::Light::HEADLIGHT>();
                            const dev::Light& backLight = dev::Factory<dev::Light>::get<dev::Light::BACKLIGHT>();

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
