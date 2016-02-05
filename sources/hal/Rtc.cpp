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

#include "Rtc.h"
#include "stm32f30x.h"
#include "stm32f30x_pwr.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR |
                                                        ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Rtc;

std::time_t Rtc::to_time_t(const time_point& __t) noexcept
{
    return std::time_t(std::chrono::duration_cast<std::chrono::seconds>(__t.time_since_epoch()).count());
}

Rtc::time_point Rtc::from_time_t(std::time_t __t) noexcept
{
    return std::chrono::time_point_cast<Rtc::duration>(time_point(std::chrono::seconds(__t)));
}

Rtc::time_point Rtc::now() noexcept
{
    RTC_TimeTypeDef time;
    RTC_DateTypeDef date;

    RTC_GetTime(RTC_Format_BIN, &time);
    RTC_GetDate(RTC_Format_BIN, &date);

    std::tm timeDate {
        time.RTC_Seconds,
        time.RTC_Minutes,
        time.RTC_Hours,
        date.RTC_Date,
        date.RTC_Month - DIFF_BETWEEN_TIMEPOINT_AND_RTC_TIME_MONTH,
        date.RTC_Year + DIFF_BETWEEN_TIMEPOINT_AND_RTC_TIME_YEAR,
        date.RTC_WeekDay - DIFF_BETWEEN_TIMEPOINT_AND_RTC_TIME_WDAY,
        0,
        1
    };

    return from_time_t(mktime(&timeDate));
}

void Rtc::set(const time_point& time) noexcept
{
    std::time_t now = to_time_t(time);
    std::tm* timeDate = localtime(&now);

    if (timeDate == nullptr) {
        Trace(ZONE_ERROR, "Internal error");
        return;
    }

    RTC_TimeTypeDef rtcTime;
    RTC_DateTypeDef rtcDate;
    rtcTime.RTC_Seconds = timeDate->tm_sec;
    rtcTime.RTC_Minutes = timeDate->tm_min;
    rtcTime.RTC_Hours = timeDate->tm_hour;
    rtcDate.RTC_Date = timeDate->tm_mday;
    rtcDate.RTC_Month = timeDate->tm_mon;
    rtcDate.RTC_Year = timeDate->tm_year;
    rtcDate.RTC_WeekDay = timeDate->tm_wday;

    rtcDate.RTC_Month += DIFF_BETWEEN_TIMEPOINT_AND_RTC_TIME_MONTH;
    rtcDate.RTC_Year -= DIFF_BETWEEN_TIMEPOINT_AND_RTC_TIME_YEAR;
    rtcDate.RTC_WeekDay += DIFF_BETWEEN_TIMEPOINT_AND_RTC_TIME_WDAY;

    set(rtcTime, rtcDate);
}

void Rtc::set(RTC_TimeTypeDef& rtcTime, RTC_DateTypeDef& rtcDate) noexcept
{
    RTC_WriteProtectionCmd(DISABLE);
    RTC_EnterInitMode();

    RTC_SetTime(RTC_Format_BIN, &rtcTime);
    RTC_SetDate(RTC_Format_BIN, &rtcDate);

    RTC_ExitInitMode();
    RTC_WriteProtectionCmd(ENABLE);
}

void Rtc::completeReset(void) const
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    PWR_BackupAccessCmd(ENABLE);

    RCC_BackupResetCmd(ENABLE);
    RCC_BackupResetCmd(DISABLE);
    if (mClockSource != RCC_RTCCLKSource_HSE_Div32) {
        PWR_BackupAccessCmd(DISABLE);
    }
}

void Rtc::initialize() const
{
    this->completeReset();

    if (mClockSource == RCC_RTCCLKSource_LSI) {
        RCC_LSICmd(ENABLE);
        while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) {}
    }
    RCC_RTCCLKConfig(mClockSource);
    RCC_RTCCLKCmd(ENABLE);

    RTC_WriteProtectionCmd(DISABLE);

    RTC_WaitForSynchro();

    RTC_Init(&mConfiguration);
    RTC_WriteProtectionCmd(ENABLE);
}

constexpr const std::array<const Rtc, Rtc::__ENUM__SIZE> hal::Factory<Rtc>::Container;
