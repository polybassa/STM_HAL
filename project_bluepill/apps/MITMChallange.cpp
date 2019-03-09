// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "TestGpio.h"
#include "Gpio.h"
#include "trace.h"
#include "Can.h"
#include "TestCan.h"
#include <cstring>

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

const os::TaskEndless MITMChallange("MITMChallange_Test",
                                    1024, os::Task::Priority::HIGH, [](const bool&){
                                    constexpr const hal::Can& can = hal::Factory<hal::Can>::get<hal::Can::MAINCAN>();
                                    constexpr const hal::Gpio& out = hal::Factory<hal::Gpio>::get<hal::Gpio::LED>();
                                    out = true;
                                    Trace(ZONE_INFO, "Hello MITM Challange\r\n");
                                    while (true) {
                                        CanTxMsg msg;

                                        for (int i = 0; i < 40; i++) {
                                            os::ThisTask::sleep(std::chrono::milliseconds(300));
                                            if (i != 19) {
                                                msg.StdId = 0x700;
                                                msg.IDE = 0;
                                                msg.RTR = 0;
                                                msg.DLC = 8;
                                                for (int j = 0; j < 8; j++) {
                                                    msg.Data[j] = 'a' + j;
                                                }
                                                auto ret = can.send(msg);

                                                os::ThisTask::sleep(std::chrono::milliseconds(300));
                                                continue;
                                            }
                                            msg.StdId = 0x700;
                                            msg.IDE = 0;
                                            msg.RTR = 0;
                                            msg.DLC = 6;
                                            msg.Data[0] = '~';
                                            msg.Data[1] = 'L';
                                            msg.Data[2] = 'i';
                                            msg.Data[3] = 'g';
                                            msg.Data[4] = 'h';
                                            msg.Data[5] = 't';
                                            auto ret = can.send(msg);
                                            Trace(ZONE_INFO, "sent on MainCan %d\r\n", ret);
                                            os::ThisTask::sleep(std::chrono::milliseconds(300));
                                            CanRxMsg rxmsg;
                                            std::memset(&rxmsg, 0, sizeof(rxmsg));
                                            auto retReceive = can.receive(rxmsg);
                                            if ((rxmsg.Data[0] == 'L') &&
                                                (rxmsg.Data[1] == 'i') &&
                                                (rxmsg.Data[2] == 'g') &&
                                                (rxmsg.Data[3] == 'h') &&
                                                (rxmsg.Data[4] == 't') &&
                                                (rxmsg.StdId == 0x520))
                                            {
                                                out = false;
                                            }
                                            Trace(ZONE_INFO, "received %d\r\n", retReceive);
                                            Trace(ZONE_INFO, "Data %s\r\n", rxmsg.Data);
                                        }
                                    }
    });
