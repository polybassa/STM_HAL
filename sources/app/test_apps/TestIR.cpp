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

#include "TestIR.h"
#include "trace.h"
#include "Usart.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

os::TaskEndless irTest("IR_Test", 2048, os::Task::Priority::LOW, [](const bool&){
                       constexpr const hal::Usart& uart = hal::Factory<hal::Usart>::get<hal::Usart::MSCOM_IF>();

                       os::ThisTask::sleep(std::chrono::milliseconds(5));

//                           uart.enableNonBlockingReceive([&](unsigned char byte){
//                                                             Trace(ZONE_INFO, "%c", byte);
//                                                         });

                       while (true) {
                           const uint8_t str[] = "HELLO_WORLD";
                           uart.send((uint8_t const* const)str, sizeof(str));

                           os::ThisTask::sleep(std::chrono::milliseconds(5));
                       }
    });
