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

#include "TestModem.h"
#include "Gpio.h"
#include "Usart.h"
#include "UsartWithDma.h"
#include "trace.h"
#include "ModemDriver.h"
#include "AT_Parser.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

os::TaskEndless modemTest("MODEM_Test", 2048, os::Task::Priority::LOW, [] (const bool&){
                              constexpr const hal::Usart& debug =
                                  hal::Factory<hal::Usart>::get<hal::Usart::DEBUG_IF>();

                              auto modem = new app::ModemDriver(
                                                                hal::Factory<hal::UsartWithDma>::get<hal::Usart::
                                                                                                     MODEM_COM>(),
                                                                hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_RESET>(),
                                                                hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_POWER>(),
                                                                hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_SUPPLY>());

                              auto callback = [&](std::string_view data)
                              {
                                  auto str = std::string(data.data(), data.length());
                                  Trace(ZONE_INFO, "Received %s\r\n", str.c_str());
                              };
                              modem->registerReceiveCallback(callback);
                              os::ThisTask::sleep(std::chrono::milliseconds(300));
                              os::ThisTask::sleep(std::chrono::milliseconds(300));
                              os::ThisTask::sleep(std::chrono::milliseconds(300));
                              Trace(ZONE_INFO, "Startup\r\n");

                              while (true) {
                                  os::ThisTask::sleep(std::chrono::milliseconds(300));
                                  modem->send(std::string_view("HiHI  "));

                                  os::ThisTask::sleep(std::chrono::milliseconds(300));

                                  modem->send(std::string_view("HUHU  "));
                                  Trace(ZONE_INFO, "loop\r\n");

                                  os::ThisTask::sleep(std::chrono::milliseconds(300));
                              }
                          });
