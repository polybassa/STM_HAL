// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_TIMPWM_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_TIMPWM_CONFIG_DESCRIPTION_H_

enum Description {
    BUZZER = 0,
    PWM1,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_TIMPWM_CONFIG_CONTAINER_H_
#define SOURCES_PMD_TIMPWM_CONFIG_CONTAINER_H_

static constexpr const std::array<const Pwm, Pwm::__ENUM__SIZE> Container =
{ {
      Pwm(Pwm::BUZZER,
          Factory<Tim>::get<Tim::BUZZER>(),
          Pwm::CHANNEL1,
          TIM_OCInitTypeDef { TIM_OCMode_PWM2, TIM_OutputState_Enable, TIM_OutputNState_Disable, 0, TIM_OCPolarity_High,
                              TIM_OCNPolarity_Low, TIM_OCIdleState_Reset,
                              TIM_OCNIdleState_Reset}),
      Pwm(Pwm::PWM1,
          Factory<Tim>::get<Tim::BUZZER>(),
          Pwm::CHANNEL2,
          TIM_OCInitTypeDef { TIM_OCMode_PWM2, TIM_OutputState_Enable, TIM_OutputNState_Disable, 0, TIM_OCPolarity_High,
                              TIM_OCNPolarity_Low, TIM_OCIdleState_Reset,
                              TIM_OCNIdleState_Reset}),
  } };

#endif /* SOURCES_PMD_TIMPWM_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_TIMPWM_CONFIG_DESCRIPTION_H_ */
