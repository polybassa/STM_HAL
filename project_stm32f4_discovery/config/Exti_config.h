// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_EXTI_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_EXTI_CONFIG_DESCRIPTION_H_

enum Description {
    USER_BUTTON_INT,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_EXTI_CONFIG_CONTAINER_H_
#define SOURCES_PMD_EXTI_CONFIG_CONTAINER_H_

static constexpr const std::array<const Exti, Exti::__ENUM__SIZE> Container =
{ {
      Exti(Exti::USER_BUTTON_INT,
           Factory<Gpio>::get<Gpio::USER_BUTTON>(),
           Factory<Nvic>::get<Nvic::USER_BUTTON_EXTI>(),
           EXTI_Trigger_Rising),
  }};
#endif /* SOURCES_PMD_EXTI_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_EXTI_CONFIG_DESCRIPTION_H_ */
