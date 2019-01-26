// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */
#include <cstring>
#include "trace.h"
#include "Can.h"
#include "TestCan.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

const os::TaskEndless canTest("Can_Test",
                              1024, os::Task::Priority::HIGH, [](const bool&){
                              constexpr const hal::Can& can = hal::Factory<hal::Can>::get<hal::Can::MAINCAN>();

                              Trace(ZONE_INFO, "Hallo MainCan\r\n");
                              while (true) {
                                  os::ThisTask::sleep(std::chrono::milliseconds(300));

                                  CanTxMsg msg;
                                  msg.StdId = 0x700;
                                  msg.IDE = 0;
                                  msg.RTR = 0;
                                  msg.DLC = 5;
                                  msg.Data[0] = 'D';
                                  msg.Data[1] = 'e';
                                  msg.Data[2] = 'n';
                                  msg.Data[3] = 'i';
                                  msg.Data[4] = 's';

                                  auto ret = can.send(msg);

                                  Trace(ZONE_INFO, "sent on  MainCan %d\r\n", ret);

                                  os::ThisTask::sleep(std::chrono::milliseconds(300));

                                  CanRxMsg rxmsg;
                                  std::memset(&rxmsg, 0, sizeof(rxmsg));

                                  auto ret = can.receive(rxmsg);

                                  os::ThisTask::sleep(std::chrono::milliseconds(300));
                                  Trace(ZONE_INFO, "pending %d\r\n", can.messagePending());
                                  Trace(ZONE_INFO, "received %d\r\n", ret);
                                  Trace(ZONE_INFO, "Data %s\r\n", rxmsg.Data);
                              }
    });
