// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_TIMHALLMETER_INTERRUPTS_H_
#define SOURCES_PMD_TIMHALLMETER_INTERRUPTS_H_

#define TIM2_HALLMETER_INTERRUPT_ENABLED true
#define TIM8_HALLMETER_INTERRUPT_ENABLED true

#endif /* SOURCES_PMD_TIMHALLMETER_INTERRUPTS_H_ */

#ifndef SOURCES_PMD_TIMHALLMETER_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_TIMHALLMETER_CONFIG_DESCRIPTION_H_

enum Description {
    BLDC_METER,
    BLDC_METER_32BIT,
    __ENUM__SIZE
};

static constexpr uint32_t POLE_PAIRS = 7;

#else
#ifndef SOURCES_PMD_TIMHALLMETER_CONFIG_CONTAINER_H_
#define SOURCES_PMD_TIMHALLMETER_CONFIG_CONTAINER_H_

static constexpr const std::array<const HallMeter, HallMeter::__ENUM__SIZE> Container =
{ {
      HallMeter(HallMeter::BLDC_METER,
                Factory<Tim>::get<Tim::HALL_METER>(),
                TIM_TS_ITR3,
                TIM_ICInitTypeDef { TIM_Channel_1, TIM_ICPolarity_Rising, TIM_ICSelection_TRC, TIM_ICPSC_DIV1, 0x00}
                ),
      HallMeter(HallMeter::BLDC_METER_32BIT,
                Factory<Tim>::get<Tim::HALL_METER_32BIT>(),
                TIM_TS_ITR2,
                TIM_ICInitTypeDef { TIM_Channel_1, TIM_ICPolarity_Rising, TIM_ICSelection_TRC, TIM_ICPSC_DIV1, 0x00}
                )
  } };

#endif /* SOURCES_PMD_TIMHALLMETER_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_TIMHALLMETER_CONFIG_DESCRIPTION_H_ */
