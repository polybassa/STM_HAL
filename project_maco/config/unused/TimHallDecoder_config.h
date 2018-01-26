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

#ifndef SOURCES_PMD_TIMHALLDECODER_INTERRUPTS_H_
#define SOURCES_PMD_TIMHALLDECODER_INTERRUPTS_H_

#define HALLDECODER_TIM3_INTERRUPT_ENABLED false
#define HALLDECODER_TIM4_INTERRUPT_ENABLED true

#endif /* SOURCES_PMD_TIMHALLDECODER_INTERRUPTS_H_ */

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
