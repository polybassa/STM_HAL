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

#ifndef SOURCES_PMD_TIM_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_TIM_CONFIG_DESCRIPTION_H_

enum Description {
    HBRIDGE,
    BUZZER,
    HALL_DECODER,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_TIM_CONFIG_CONTAINER_H_
#define SOURCES_PMD_TIM_CONFIG_CONTAINER_H_

static constexpr const std::array<const Tim, Tim::__ENUM__SIZE + 1> Container =
{ {
      Tim(Tim::HBRIDGE,
          TIM1_BASE,
          TIM_TimeBaseInitTypeDef {0, TIM_CounterMode_Up, Tim::HALFBRIDGE_PERIODE, TIM_CKD_DIV1, 0}),
      Tim(Tim::BUZZER,
          TIM2_BASE,
          TIM_TimeBaseInitTypeDef {0, TIM_CounterMode_Up, Tim::BUZZER_PWM_PERIODE, TIM_CKD_DIV1, 0}),
      Tim(Tim::HALL_DECODER,
          TIM3_BASE,
          TIM_TimeBaseInitTypeDef {Tim::HALL_SENSOR_PRESCALER, TIM_CounterMode_Up, 0xffff, TIM_CKD_DIV1, 0}),
      Tim(Tim::__ENUM__SIZE,
          0xffffffff,
          TIM_TimeBaseInitTypeDef {0, TIM_CounterMode_Up, 0, TIM_CKD_DIV1, 0})
  } };

static constexpr const std::array<const uint32_t, Tim::__ENUM__SIZE> Clocks =
{ {
      RCC_APB2Periph_TIM1,
      RCC_APB1Periph_TIM2,
      RCC_APB1Periph_TIM3
  } };

#endif /* SOURCES_PMD_TIM_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_TIM_CONFIG_DESCRIPTION_H_ */
