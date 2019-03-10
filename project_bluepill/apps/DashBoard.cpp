// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "Gpio.h"
#include "trace.h"
#include "Can.h"
#include "DashBoard.h"
#include <cstring>
#include "CANFrames.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

const os::TaskEndless DashBoard("Dash",
                                2048, os::Task::Priority::MEDIUM, [](const bool&){
                                constexpr const hal::Can& can = hal::Factory<hal::Can>::get<hal::Can::MAINCAN>();
                                Trace(ZONE_INFO, "Hello MITM Challange Dashboard\r\n");
                                while (true) {
                                    app::msg msg;

                                    os::ThisTask::sleep(std::chrono::milliseconds(300));

                                    msg.setType(app::msg::type::tempomat);
                                    msg.setData(0);
                                    can.send(msg);
                                    os::ThisTask::sleep(std::chrono::milliseconds(300));

                                    msg.setType(app::msg::type::ignition);
                                    msg.setData(0);
                                    can.send(msg);
                                    os::ThisTask::sleep(std::chrono::milliseconds(300));

                                    msg.setType(app::msg::type::start);
                                    msg.setData(0);
                                    can.send(msg);
                                }
    });

const os::TaskEndless DashBoardRx("DashRx",
                                  2048, os::Task::Priority::HIGH, [](const bool&) {
                                  constexpr const hal::Can& can = hal::Factory<hal::Can>::get<hal::Can::MAINCAN>();
                                  Trace(ZONE_INFO, "Hello MITM Dashboard Receive\r\n");

                                  while (true) {
                                      os::ThisTask::sleep(std::chrono::milliseconds(5));

                                      CanRxMsg rxmsg;
                                      std::memset(&rxmsg, 0, sizeof(rxmsg));
                                      auto retReceive = can.receive(rxmsg);

                                      if (retReceive) {
                                          app::msg rxm;
                                          memcpy(&rxm, &rxmsg, sizeof(rxm));

                                          switch (rxm.getType()) {
                                          case app::msg::type::speed:
                                              Trace(ZONE_INFO, "Set Speed to %d\r\n", rxm.getData());
                                              break;

                                          case app::msg::type::fuelLevel:
                                              Trace(ZONE_INFO, "Set fuelLevel to %d\r\n", rxm.getData());
                                              break;

                                          case app::msg::type::oilLevel:
                                              Trace(ZONE_INFO, "Set fuelLevel to %d\r\n", rxm.getData());
                                              break;

                                          case app::msg::type::engineTemperature:
                                              Trace(ZONE_INFO, "Set engineTemperature to %d\r\n", rxm.getData());
                                              break;

                                          case app::msg::type::engineRPM:
                                              Trace(ZONE_INFO, "Set engineRPM to %d\r\n", rxm.getData());
                                              break;

                                          case app::msg::type::malfunction:
                                              Trace(ZONE_INFO, "Set mal to %d\r\n", rxm.getData());
                                              break;
                                          }
                                      }
                                  }
    });
