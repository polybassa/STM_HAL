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

#ifndef SOURCES_PMD_GPIO_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_GPIO_CONFIG_DESCRIPTION_H_

enum Description {
    // ===PORTA===
    SWDIO,
    SWCLK,
    // ===PORTB===
    // ===PORTC===
    // ===PORTD===
    // ===PORTE===
    // ===PORTF===
    // ===PORTG===
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_GPIO_CONFIG_CONTAINER_H_
#define SOURCES_PMD_GPIO_CONFIG_CONTAINER_H_

static constexpr const std::array<const Gpio, Gpio::__ENUM__SIZE + 1> Container =
{ {
      // ===================PORTA=================
      Gpio(Gpio::SWDIO,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_13, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
           GPIO_PinSource13),
      Gpio(Gpio::SWCLK,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_14, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
           GPIO_PinSource14),
      // ===================PORTB=================

      // ===================PORTC=================

      // ===================PORTD=================

      // ===================PORTE=================

      // ===================PORTF=================

      // ===================PORTG=================
      Gpio(Gpio::__ENUM__SIZE,
           GPIOD_BASE,
           GPIO_InitTypeDef { GPIO_Pin_14, GPIO_Speed_50MHz, GPIO_Mode_AF_PP })
  }};

#endif /* SOURCES_PMD_GPIO_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_GPIO_CONFIG_DESCRIPTION_H_ */
