// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include <cstring>
#include "trace.h"
#include "Can.h"
#include "IsoTp.h"
#include "TestIsoTp.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

const os::TaskEndless canTest("ISOTP_Test",
                              1024, os::Task::Priority::HIGH, [](const bool&){
                              constexpr const hal::Can& can = hal::Factory<hal::Can>::get<hal::Can::MAINCAN>();
                              app::ISOTP isotp(can, 0x734, 0x456);

                              Trace(ZONE_INFO, "Hallo ISOTP Test\r\n");
                              while (true) {
                                  os::ThisTask::sleep(std::chrono::milliseconds(300));

                                  std::string_view message = "EINS";

                                  auto sent = isotp.send_Message(message, std::chrono::milliseconds(500));
                                  Trace(ZONE_INFO, "pending %d\r\n", sent);

                                  char buffer[50];
                                  auto received = isotp.receive_Message(buffer,
                                                                        sizeof(buffer),
                                                                        std::chrono::milliseconds(500));
                                  Trace(ZONE_INFO, "pending received bytes= %d\r\n", received);
                              }
    });
