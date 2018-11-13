// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_ADC_CHANNEL_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_ADC_CHANNEL_CONFIG_DESCRIPTION_H_

enum Description {
    BATTERY_U,
    BATTERY_I,
    NTC_BATTERY,
    DMS,
    INTERNAL_TEMP,
    NTC_MOTOR,
    NTC_FET,
    IB_FB,
    I_TOTAL_FB,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_ADC_CHANNEL_CONFIG_CONTAINER_H_
#define SOURCES_PMD_ADC_CHANNEL_CONFIG_CONTAINER_H_

static constexpr const std::array<const Adc::Channel,
                                  Adc::Channel::Description::__ENUM__SIZE> Container =
{ {
      Adc::Channel(Adc::Channel::BATTERY_U,
                   Factory<Adc>::get<Adc::Description::PMD_ADC1>(),
                   ADC_Channel_7,
                   ADC_SampleTime_61Cycles5,
                   std::chrono::milliseconds(5),
                   3),
      Adc::Channel(Adc::Channel::BATTERY_I,
                   Factory<Adc>::get<Adc::Description::PMD_ADC1>(),
                   ADC_Channel_8,
                   ADC_SampleTime_61Cycles5,
                   std::chrono::milliseconds(5)),
      Adc::Channel(Adc::Channel::NTC_BATTERY,
                   Factory<Adc>::get<Adc::Description::PMD_ADC1>(),
                   ADC_Channel_1,
                   ADC_SampleTime_61Cycles5,
                   std::chrono::milliseconds(30)),
      Adc::Channel(Adc::Channel::DMS,
                   Factory<Adc>::get<Adc::Description::PMD_ADC2>(),
                   ADC_Channel_9,
                   ADC_SampleTime_61Cycles5,
                   std::chrono::milliseconds(30)),
      Adc::Channel(Adc::Channel::INTERNAL_TEMP,
                   Factory<Adc>::get<Adc::Description::PMD_ADC1>(),
                   ADC_Channel_TempSensor,
                   ADC_SampleTime_61Cycles5,
                   std::chrono::milliseconds(100)),
      Adc::Channel(Adc::Channel::NTC_MOTOR,
                   Factory<Adc>::get<Adc::Description::PMD_ADC1>(),
                   ADC_Channel_2,
                   ADC_SampleTime_61Cycles5,
                   std::chrono::milliseconds(100)),
      Adc::Channel(Adc::Channel::NTC_FET,
                   Factory<Adc>::get<Adc::Description::PMD_ADC1>(),
                   ADC_Channel_6,
                   ADC_SampleTime_61Cycles5,
                   std::chrono::milliseconds(100)),
      Adc::Channel(Adc::Channel::IB_FB,
                   Factory<Adc>::get<Adc::Description::PMD_ADC3>(),
                   ADC_Channel_13,
                   ADC_SampleTime_4Cycles5,
                   std::chrono::milliseconds(0)),
      Adc::Channel(Adc::Channel::I_TOTAL_FB,
                   Factory<Adc>::get<Adc::Description::PMD_ADC4>(),
                   ADC_Channel_1,
                   ADC_SampleTime_4Cycles5,
                   std::chrono::milliseconds(0),
                   3),
  } };

#endif /* SOURCES_PMD_ADC_CHANNEL_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_ADC_CHANNEL_CONFIG_DESCRIPTION_H_ */
