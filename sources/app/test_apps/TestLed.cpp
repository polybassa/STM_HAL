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

os::TaskEndless ledTest("LED_Test", 1024, 4, [](const bool&){
    const dev::Light& headLight = dev::Factory<dev::Light>::get<dev::Light::HEADLIGHT>();
    const dev::Light& backLight = dev::Factory<dev::Light>::get<dev::Light::BACKLIGHT>();

    uint8_t temp = 0;

    os::ThisTask::sleep(std::chrono::milliseconds(500));
    backLight.setColor({0, 0, 255});
    os::ThisTask::sleep(std::chrono::milliseconds(500));
    backLight.setColor({0, 255, 255});
    os::ThisTask::sleep(std::chrono::milliseconds(500));
    backLight.setColor({0, 255, 0});
    os::ThisTask::sleep(std::chrono::milliseconds(500));
    backLight.setColor({255, 255, 0});
    os::ThisTask::sleep(std::chrono::milliseconds(500));
    backLight.setColor({255, 0, 0});
    os::ThisTask::sleep(std::chrono::milliseconds(500));
    backLight.setColor({255, 0, 255});

    while (true) {
        headLight.setColor({temp, temp, temp});
        backLight.setColor({temp, temp, temp});

        temp++;

        os::ThisTask::sleep(std::chrono::milliseconds(20));
    }
});
