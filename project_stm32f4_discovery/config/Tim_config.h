// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_TIM_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_TIM_CONFIG_DESCRIPTION_H_

enum Description {
    MOTOR_CONTROL_TIM,
    ROTARY_ENCODER,
    __ENUM__SIZE
};

static constexpr const uint32_t HALFBRIDGE_PERIODE = 7200;
static constexpr const uint32_t HALL_SENSOR_PRESCALER = 179;
static constexpr const uint32_t BUZZER_PWM_PERIODE = 72000;
static constexpr const uint32_t COMPARE_VALUE_PWM_28kHz = 3000;

#else
#ifndef SOURCES_PMD_TIM_CONFIG_CONTAINER_H_
#define SOURCES_PMD_TIM_CONFIG_CONTAINER_H_

static constexpr const std::array<const Tim, Tim::__ENUM__SIZE + 1> Container =
{ {
      Tim(Tim::MOTOR_CONTROL_TIM,
          TIM1_BASE,
          TIM_TimeBaseInitTypeDef {0, TIM_CounterMode_CenterAligned3, Tim::COMPARE_VALUE_PWM_28kHz, TIM_CKD_DIV1,
                                   1 /*0x50*/},
          RCC_APB2Periph_TIM1,
          TIM_BDTRInitTypeDef {TIM_OSSRState_Disable, TIM_OSSIState_Disable, TIM_LOCKLevel_OFF, 0x1F,
                               TIM_Break_Disable, TIM_BreakPolarity_Low, TIM_AutomaticOutput_Disable}), // all std values, except the dead time),
      Tim(Tim::ROTARY_ENCODER,
          TIM4_BASE,
          TIM_TimeBaseInitTypeDef {0, TIM_CounterMode_Up, 0x3fff, TIM_CKD_DIV1, 0},
          RCC_APB1Periph_TIM4),
      Tim(Tim::__ENUM__SIZE,
          0xffffffff,
          TIM_TimeBaseInitTypeDef {0, TIM_CounterMode_Up, 0, TIM_CKD_DIV1, 0}),
  } };

#endif /* SOURCES_PMD_TIM_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_TIM_CONFIG_DESCRIPTION_H_ */
