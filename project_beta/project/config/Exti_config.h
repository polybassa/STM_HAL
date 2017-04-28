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

#ifndef SOURCES_PMD_EXTI_INTERRUPTS_H_
#define SOURCES_PMD_EXTI_INTERRUPTS_H_

#define EXTI0_INTERRUPT_ENABLED false
#define EXTI1_INTERRUPT_ENABLED true
#define EXTI2_INTERRUPT_ENABLED false
#define EXTI3_INTERRUPT_ENABLED false
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
    IMU_INT,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_EXTI_CONFIG_CONTAINER_H_
#define SOURCES_PMD_EXTI_CONFIG_CONTAINER_H_

static constexpr const std::array<const Exti, Exti::__ENUM__SIZE> Container =
{ {
      Exti(Exti::IMU_INT,
           Factory<Gpio>::get<Gpio::MEMS_INT>(),
           EXTI_Trigger_Rising)
  }};
#endif /* SOURCES_PMD_EXTI_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_EXTI_CONFIG_DESCRIPTION_H_ */
