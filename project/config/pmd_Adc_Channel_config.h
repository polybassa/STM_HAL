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
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_ADC_CHANNEL_CONFIG_CONTAINER_H_
#define SOURCES_PMD_ADC_CHANNEL_CONFIG_CONTAINER_H_

static constexpr const std::array<const Adc::Channel,
                                  Adc::Channel::Description::__ENUM__SIZE> ChannelContainer =
{ {
      Adc::Channel(Adc::Channel::BATTERY_U,
                   Adc::PMD_ADC1,
                   ADC_Channel_6,
                   ADC_SampleTime_181Cycles5,
                   std::chrono::milliseconds(5)),
      Adc::Channel(Adc::Channel::BATTERY_I,
                   Adc::PMD_ADC1,
                   ADC_Channel_7,
                   ADC_SampleTime_61Cycles5,
                   std::chrono::milliseconds(5)),
      Adc::Channel(Adc::Channel::NTC_BATTERY,
                   Adc::PMD_ADC1,
                   ADC_Channel_8,
                   ADC_SampleTime_601Cycles5,
                   std::chrono::milliseconds(100)),
      Adc::Channel(Adc::Channel::DMS,
                   Adc::PMD_ADC1,
                   ADC_Channel_9,
                   ADC_SampleTime_601Cycles5,
                   std::chrono::milliseconds(30)),
      Adc::Channel(Adc::Channel::INTERNAL_TEMP,
                   Adc::PMD_ADC1,
                   ADC_Channel_TempSensor,
                   ADC_SampleTime_601Cycles5,
                   std::chrono::milliseconds(100)),
      Adc::Channel(Adc::Channel::NTC_MOTOR,
                   Adc::PMD_ADC2,
                   ADC_Channel_5,
                   ADC_SampleTime_601Cycles5,
                   std::chrono::milliseconds(100)),
      Adc::Channel(Adc::Channel::NTC_FET,
                   Adc::PMD_ADC2,
                   ADC_Channel_11,
                   ADC_SampleTime_601Cycles5,
                   std::chrono::milliseconds(100)),
  } };

#endif /* SOURCES_PMD_ADC_CHANNEL_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_ADC_CHANNEL_CONFIG_DESCRIPTION_H_ */
