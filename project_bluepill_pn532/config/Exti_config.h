// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_EXTI_INTERRUPTS_H_
#define SOURCES_PMD_EXTI_INTERRUPTS_H_

#define EXTI0_INTERRUPT_ENABLED false
#define EXTI1_INTERRUPT_ENABLED false
#define EXTI2_INTERRUPT_ENABLED false
#define EXTI3_INTERRUPT_ENABLED true
#define EXTI4_INTERRUPT_ENABLED false
#define EXTI5_INTERRUPT_ENABLED false
#define EXTI6_INTERRUPT_ENABLED false
#define EXTI7_INTERRUPT_ENABLED false
#define EXTI8_INTERRUPT_ENABLED false
#define EXTI9_INTERRUPT_ENABLED false
#define EXTI10_INTERRUPT_ENABLED false
#define EXTI11_INTERRUPT_ENABLED false
#define EXTI12_INTERRUPT_ENABLED false
#define EXTI13_INTERRUPT_ENABLED false
#define EXTI14_INTERRUPT_ENABLED false
#define EXTI15_INTERRUPT_ENABLED false

#endif /* SOURCES_PMD_EXTI_INTERRUPTS_H_ */

#ifndef SOURCES_PMD_EXTI_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_EXTI_CONFIG_DESCRIPTION_H_

enum Description {
    TRIGGER,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_EXTI_CONFIG_CONTAINER_H_
#define SOURCES_PMD_EXTI_CONFIG_CONTAINER_H_

static constexpr const std::array<const Exti, Exti::__ENUM__SIZE> Container =
{ {
      Exti(Exti::TRIGGER,
           {EXTI_Line3, EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling, DISABLE},
           GPIO_PortSourceGPIOA,
           GPIO_PinSource3)
  }};
#endif /* SOURCES_PMD_EXTI_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_EXTI_CONFIG_DESCRIPTION_H_ */
