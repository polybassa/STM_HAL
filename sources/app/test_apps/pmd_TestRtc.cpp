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

#include "pmd_TestRtc.h"
#include "pmd_Rtc.h"
#include "pmd_DebugInterface.h"
#include <chrono>

using dev::DebugInterface;
using hal::Factory;
using hal::Rtc;
using hal::Usart;
using os::TaskEndless;

TaskEndless rtcTest("RTC_Check", 1024, 4, [](const bool&) {
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
