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
