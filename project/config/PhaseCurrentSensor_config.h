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

#ifndef SOURCES_PMD_PHASECURRENTSENSOR_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_PHASECURRENTSENSOR_CONFIG_DESCRIPTION_H_

enum Description {
    I_TOTAL_FB,
    __ENUM__SIZE
};

static constexpr const uint32_t NUMBER_OF_MEASUREMENTS_FOR_AVG = 20;
static constexpr const float SHUNT_RESISTANCE = 0.002; // Ohm
static constexpr const float MEASUREMENT_GAIN = 40.0;

#else
#ifndef SOURCES_PMD_PHASECURRENTSENSOR_CONFIG_CONTAINER_H_
#define SOURCES_PMD_PHASECURRENTSENSOR_CONFIG_CONTAINER_H_

static constexpr std::array<const PhaseCurrentSensor, PhaseCurrentSensor::__ENUM__SIZE> Container =
{ {
      PhaseCurrentSensor(PhaseCurrentSensor::I_TOTAL_FB,
                         hal::Factory<hal::HalfBridge>::get<hal::HalfBridge::BLDC_PWM>(),
                         hal::Factory<hal::AdcWithDma>::get<hal::Adc::Channel::I_TOTAL_FB>(),
                         TIM_OCInitTypeDef { TIM_OCMode_PWM2, TIM_OutputState_Enable, TIM_OutputNState_Disable, 1,
                                             TIM_OCPolarity_High, TIM_OCNPolarity_High, TIM_OCIdleState_Reset,
                                             TIM_OCNIdleState_Reset})
  } };

#endif /* SOURCES_PMD_PHASECURRENTSENSOR_CONFIG_DESCRIPTION_H_ */
#endif /* SOURCES_PMD_PHASECURRENTSENSOR_CONFIG_DESCRIPTION_H_ */
