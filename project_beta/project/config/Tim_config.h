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
    HALL_METER_32BIT,
    HALL_DECODER,
    HALL_METER,
    BUZZER,
    HBRIDGE,
    __ENUM__SIZE
};

static constexpr const uint32_t HALFBRIDGE_PERIODE = 7200;
static constexpr const uint32_t HALL_SENSOR_PRESCALER = 179;
static constexpr const uint32_t BUZZER_PWM_PERIODE = 72000;

#else
#ifndef SOURCES_PMD_TIM_CONFIG_CONTAINER_H_
#define SOURCES_PMD_TIM_CONFIG_CONTAINER_H_

static constexpr const std::array<const Tim, Tim::__ENUM__SIZE + 1> Container =
{ {
      Tim(Tim::HALL_METER_32BIT,
          TIM2_BASE,
          TIM_TimeBaseInitTypeDef {0, TIM_CounterMode_Up, 0xffffffff, TIM_CKD_DIV1, 0}),
      Tim(Tim::HALL_DECODER,
          TIM4_BASE,
          TIM_TimeBaseInitTypeDef {Tim::HALL_SENSOR_PRESCALER, TIM_CounterMode_Up, 0xffff, TIM_CKD_DIV1, 0}),
      Tim(Tim::HALL_METER,
          TIM8_BASE,
          TIM_TimeBaseInitTypeDef {Tim::HALL_SENSOR_PRESCALER, TIM_CounterMode_Up, 0xffff, TIM_CKD_DIV1, 0}),
      Tim(Tim::BUZZER,
          TIM15_BASE,
          TIM_TimeBaseInitTypeDef {0, TIM_CounterMode_Up, Tim::BUZZER_PWM_PERIODE, TIM_CKD_DIV1, 0}),
      Tim(Tim::HBRIDGE,
          TIM20_BASE,
          TIM_TimeBaseInitTypeDef {0, TIM_CounterMode_Up, Tim::HALFBRIDGE_PERIODE, TIM_CKD_DIV1, 0}),
      Tim(Tim::__ENUM__SIZE,
          0xffffffff,
          TIM_TimeBaseInitTypeDef {0, TIM_CounterMode_Up, 0, TIM_CKD_DIV1, 0}),
  } };

static constexpr const std::array<const uint32_t, Tim::__ENUM__SIZE> Clocks =
{ {
      RCC_APB1Periph_TIM2,
      RCC_APB1Periph_TIM4,
      RCC_APB2Periph_TIM8,
      RCC_APB2Periph_TIM15,
      RCC_APB2Periph_TIM20,
  } };

#endif /* SOURCES_PMD_TIM_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_TIM_CONFIG_DESCRIPTION_H_ */
