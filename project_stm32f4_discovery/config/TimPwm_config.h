// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_TIMPWM_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_TIMPWM_CONFIG_DESCRIPTION_H_

enum Description {
    MOTOR_CONTROL_PWM_A,
    MOTOR_CONTROL_PWM_B,
    MOTOR_CONTROL_PWM_C,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_TIMPWM_CONFIG_CONTAINER_H_
#define SOURCES_PMD_TIMPWM_CONFIG_CONTAINER_H_

static constexpr const std::array<const Pwm, Pwm::__ENUM__SIZE> Container =
{ {
      Pwm(Pwm::MOTOR_CONTROL_PWM_A,
          Factory<Tim>::get<Tim::MOTOR_CONTROL_TIM>(),
          Pwm::CHANNEL1,
          TIM_OCInitTypeDef { TIM_OCMode_PWM1, TIM_OutputState_Enable, TIM_OutputNState_Enable, 0,
                              TIM_OCPolarity_High, TIM_OCNPolarity_Low, TIM_OCIdleState_Reset, TIM_OCNIdleState_Set}),

      Pwm(Pwm::MOTOR_CONTROL_PWM_B,
          Factory<Tim>::get<Tim::MOTOR_CONTROL_TIM>(),
          Pwm::CHANNEL2,
          TIM_OCInitTypeDef { TIM_OCMode_PWM1, TIM_OutputState_Enable, TIM_OutputNState_Enable, 0,
                              TIM_OCPolarity_High, TIM_OCNPolarity_Low, TIM_OCIdleState_Reset, TIM_OCNIdleState_Set}),

      Pwm(Pwm::MOTOR_CONTROL_PWM_C,
          Factory<Tim>::get<Tim::MOTOR_CONTROL_TIM>(),
          Pwm::CHANNEL3,
          TIM_OCInitTypeDef { TIM_OCMode_PWM1, TIM_OutputState_Enable, TIM_OutputNState_Enable, 0,
                              TIM_OCPolarity_High, TIM_OCNPolarity_Low, TIM_OCIdleState_Reset, TIM_OCNIdleState_Set}),
  } };

#endif /* SOURCES_PMD_TIMPWM_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_TIMPWM_CONFIG_DESCRIPTION_H_ */
