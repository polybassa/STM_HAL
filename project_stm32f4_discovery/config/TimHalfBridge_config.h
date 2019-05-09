// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_TIMHALFBRIDGE_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_TIMHALFBRIDGE_CONFIG_DESCRIPTION_H_

enum Description {
    BLDC_PWM,
    __ENUM__SIZE
};

#define ACTIVEFREEWHEELING 1

static constexpr const uint32_t DEFAULT_DEADTIME = 50;
static constexpr const uint32_t MINIMAL_PWM_IN_MILL = 35;
static constexpr const uint32_t MAXIMAL_PWM_IN_MILL = 1000;

#else
#ifndef SOURCES_PMD_TIMHALFBRIDGE_CONFIG_CONTAINER_H_
#define SOURCES_PMD_TIMHALFBRIDGE_CONFIG_CONTAINER_H_

static constexpr const std::array<const HalfBridge, HalfBridge::__ENUM__SIZE> Container =
{ {
      HalfBridge(HalfBridge::BLDC_PWM,
                 Factory<Tim>::get<Tim::HBRIDGE>(),
                 TIM_TS_ITR2,
                 TIM_OCInitTypeDef { TIM_OCMode_PWM1, TIM_OutputState_Enable, TIM_OutputNState_Enable, 0,
                                     TIM_OCPolarity_High, TIM_OCNPolarity_High, TIM_OCIdleState_Reset,
                                     TIM_OCNIdleState_Reset},
                 TIM_BDTRInitTypeDef { TIM_OSSRState_Enable, TIM_OSSIState_Enable, TIM_LOCKLevel_OFF,
                                       HalfBridge::DEFAULT_DEADTIME, TIM_Break_Disable, TIM_BreakPolarity_Low,
                                       TIM_AutomaticOutput_Enable})
  } };

#endif /* SOURCES_PMD_TIMHALFBRIDGE_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_TIMHALFBRIDGE_CONFIG_DESCRIPTION_H_ */
