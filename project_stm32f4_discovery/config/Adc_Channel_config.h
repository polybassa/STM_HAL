// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_ADC_CHANNEL_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_ADC_CHANNEL_CONFIG_DESCRIPTION_H_

enum Description {
    TEST_ADC,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_ADC_CHANNEL_CONFIG_CONTAINER_H_
#define SOURCES_PMD_ADC_CHANNEL_CONFIG_CONTAINER_H_

static constexpr const std::array<const Adc::Channel,
                                  Adc::Channel::Description::__ENUM__SIZE> Container =
{ {
      Adc::Channel(Adc::Channel::TEST_ADC,
                   Factory<Adc>::get<Adc::Description::HAL_ADC1>(),
                   ADC_Channel_11,
                   ADC_SampleTime_3Cycles, std::chrono::milliseconds(0)),
  } };

#endif /* SOURCES_PMD_ADC_CHANNEL_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_ADC_CHANNEL_CONFIG_DESCRIPTION_H_ */
