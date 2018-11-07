// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_TIMHALLDECODER_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_TIMHALLDECODER_CONFIG_DESCRIPTION_H_

enum Description {
    BLDC_DECODER,
    __ENUM__SIZE
};

static constexpr uint32_t POLE_PAIRS = 7;

#else
#ifndef SOURCES_PMD_TIMHALLDECODER_CONFIG_CONTAINER_H_
#define SOURCES_PMD_TIMHALLDECODER_CONFIG_CONTAINER_H_

static constexpr const std::array<const HallDecoder, HallDecoder::__ENUM__SIZE> Container =
{ {
      HallDecoder(HallDecoder::BLDC_DECODER,
                  Factory<Tim>::get<Tim::HALL_DECODER>(),
                  TIM_ICInitTypeDef { TIM_Channel_1, TIM_ICPolarity_Rising, TIM_ICSelection_TRC, TIM_ICPSC_DIV1, 0x0F},
                  TIM_OCInitTypeDef { TIM_OCMode_PWM2, TIM_OutputState_Disable, TIM_OutputNState_Disable, 1,
                                      TIM_OCPolarity_High, TIM_OCNPolarity_High, TIM_OCIdleState_Reset,
                                      TIM_OCNIdleState_Reset},
                  TIM_OCInitTypeDef { TIM_OCMode_PWM2, TIM_OutputState_Disable, TIM_OutputNState_Disable, 0xffff,
                                      TIM_OCPolarity_High, TIM_OCNPolarity_High, TIM_OCIdleState_Reset,
                                      TIM_OCNIdleState_Reset}
                  )
  } };

#endif /* SOURCES_PMD_TIMHALLDECODER_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_TIMHALLDECODER_CONFIG_DESCRIPTION_H_ */
