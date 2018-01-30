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
#include "Usart.h"
#include "UsartWithDma.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

os::TaskEndless gpioTest("Gpio_Test", 1024, os::Task::Priority::MEDIUM, [] (const bool&){
                             constexpr const hal::Gpio& out = hal::Factory<hal::Gpio>::get<hal::Gpio::TEST_PIN_OUT>();
                             constexpr const hal::UsartWithDma& gsm =
                                 hal::Factory<hal::UsartWithDma>::get<hal::Usart::MODEM_COM>();
                             constexpr const hal::UsartWithDma& secco =
                                 hal::Factory<hal::UsartWithDma>::get<hal::Usart::SECCO_COM>();

                             while (true) {
                                 os::ThisTask::sleep(std::chrono::milliseconds(300));
                                 out = true;
                                 os::ThisTask::sleep(std::chrono::milliseconds(300));
                                 out = false;

                                 gsm.send(reinterpret_cast<const uint8_t*>("Hello "), 6);
                                 secco.send(reinterpret_cast<const uint8_t*>("Hello "), 6);
                                 Trace(ZONE_INFO, "loop\r\n");

                                 os::ThisTask::sleep(std::chrono::milliseconds(300));
                             }
                         });
