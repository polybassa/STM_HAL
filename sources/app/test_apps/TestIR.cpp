// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

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
