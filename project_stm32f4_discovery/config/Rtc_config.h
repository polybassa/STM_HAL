// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_RTC_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_RTC_CONFIG_DESCRIPTION_H_

enum Description {
    SYSTEM_RTC,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_RTC_CONFIG_CONTAINER_H_
#define SOURCES_PMD_RTC_CONFIG_CONTAINER_H_

static constexpr const std::array<const Rtc, Rtc::__ENUM__SIZE> Container =
{ {
      Rtc(Rtc::SYSTEM_RTC, RCC_RTCCLKSource_LSI, RTC_InitTypeDef {RTC_HourFormat_24, 127, 249})
  } };

#endif /* SOURCES_PMD_RTC_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_RTC_CONFIG_DESCRIPTION_H_ */
