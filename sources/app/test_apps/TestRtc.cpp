// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "TestRtc.h"
#include "Rtc.h"
#include "DebugInterface.h"
#include <chrono>

using dev::DebugInterface;
using hal::Factory;
using hal::Rtc;
using hal::Usart;
using os::TaskEndless;

TaskEndless rtcTest("RTC_Check", 1024, 4, [](const bool&){
                    const DebugInterface terminal;
                    terminal.print("Hello TimeWorld");

                    RTC_TimeTypeDef rtcTime { 12, 00, 00, 1};
                    RTC_DateTypeDef rtcDate { RTC_Weekday_Monday, RTC_Month_September, 14, 15 };

                    Rtc::set(rtcTime, rtcDate);

                    while (true) {
                        char buffer[30];
                        auto timer = Rtc::to_time_t(Rtc::now());
                        std::tm* tm_info = std::localtime(&timer);
                        std::strftime(buffer, 26, "%Y:%m:%d %H:%M:%S\r\n", tm_info);
                        terminal.print("%s", buffer);
                        os::ThisTask::sleep(std::chrono::seconds(1));
                    }
    });
