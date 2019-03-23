// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_ADC_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_ADC_CONFIG_DESCRIPTION_H_

enum Description {
    HORROR_ADC1,
    HORROR_ADC2,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_ADC_CONFIG_CONTAINER_H_
#define SOURCES_PMD_ADC_CONFIG_CONTAINER_H_

static constexpr std::array<const AdcChannel, 2> ChannelContainer =
{{
     {   ADC1, ADC_Channel_0, 1, ADC_SampleTime_13Cycles5 },
     {   ADC2, ADC_Channel_0, 2, ADC_SampleTime_13Cycles5 }
 }};

static constexpr std::array<const Adc, Adc::Description::__ENUM__SIZE> Container =
{ {
      Adc(Adc::HORROR_ADC1,
          ADC1_BASE,
          {ADC_Mode_SlowInterl, DISABLE, DISABLE, ADC_ExternalTrigConv_None, ADC_DataAlign_Right, 1},
          ENABLE),
      Adc(Adc::HORROR_ADC2,
          ADC2_BASE,
          {ADC_Mode_SlowInterl, DISABLE, DISABLE, ADC_ExternalTrigConv_None, ADC_DataAlign_Right, 1},
          DISABLE)
  } };

#endif /* SOURCES_PMD_ADC_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_ADC_CONFIG_DESCRIPTION_H_ */
