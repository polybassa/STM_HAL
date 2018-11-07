// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_PHASECURRENTSENSOR_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_PHASECURRENTSENSOR_CONFIG_DESCRIPTION_H_

enum Description {
    I_TOTAL_FB,
    __ENUM__SIZE
};

static constexpr const uint32_t MAX_NUMBER_OF_MEASUREMENTS = 64;
static constexpr const float SHUNT_RESISTANCE = 0.002; // Ohm
static constexpr const float MEASUREMENT_GAIN = 22.1;
static constexpr const float FILTERWIDTH = 128;

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
