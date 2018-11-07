// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_RTC_H_
#define SOURCES_PMD_RTC_H_

#include <cstdint>
#include <array>
#include <chrono>
#include <ctime>
#include "stm32f30x_rtc.h"
#include "stm32f30x_rcc.h"
#include "hal_Factory.h"

namespace hal
{
struct Rtc {
#include "Rtc_config.h"

    Rtc() = delete;
    Rtc(const Rtc&) = delete;
    Rtc(Rtc&&) = default;
    Rtc& operator=(const Rtc&) = delete;
    Rtc& operator=(Rtc&&) = delete;

    /* Definitions for std::chrono::clock */
    typedef std::chrono::seconds duration;
    typedef duration::rep rep;
    typedef duration::period period;
    typedef std::chrono::time_point<Rtc> time_point;
    static const bool is_steady = true;

    static std::time_t to_time_t(const time_point& __t) noexcept;
    static time_point from_time_t(std::time_t __t) noexcept;

    static time_point now() noexcept;
    static void set(const time_point& time) noexcept;
    static void set(RTC_TimeTypeDef& rtcTime, RTC_DateTypeDef& rtcDate) noexcept;

private:
    constexpr Rtc(const enum Description& desc,
                  const uint32_t          clkSource,
                  const RTC_InitTypeDef&  conf) :
        mDescription(desc), mClockSource(clkSource), mConfiguration(conf) {}

    const enum Description mDescription;
    const uint32_t mClockSource;
    const RTC_InitTypeDef mConfiguration;

    void completeReset(void) const;
    void initialize(void) const;

    static const uint8_t DIFF_BETWEEN_TIMEPOINT_AND_RTC_TIME_MONTH = 1;
    static const uint8_t DIFF_BETWEEN_TIMEPOINT_AND_RTC_TIME_YEAR = 100;
    static const uint8_t DIFF_BETWEEN_TIMEPOINT_AND_RTC_TIME_WDAY = 1;

    friend class Factory<Rtc>;
};

template<>
class Factory<Rtc>
{
#include "Rtc_config.h"

    template<enum Rtc::Description index>
    static constexpr const Rtc& get(void)
    {
        static_assert(IS_RTC_HOUR_FORMAT(Container[index].mConfiguration.RTC_HourFormat), "Invalid");
        static_assert(IS_RTC_ASYNCH_PREDIV(Container[index].mConfiguration.RTC_AsynchPrediv), "Invalid");
        static_assert(IS_RTC_SYNCH_PREDIV(Container[index].mConfiguration.RTC_SynchPrediv), "Invalid");
        static_assert(IS_RCC_RTCCLK_SOURCE(Container[index].mClockSource), "Invalid");

        static_assert(index != Rtc::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    Factory(void)
    {
        get<Rtc::SYSTEM_RTC>().initialize();
    }

public:
    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_RTC_H_ */
