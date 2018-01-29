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
    USART2_TX,
    USART2_RX,
    USART1_TX,
    USART1_RX,
    SWDIO,
    SWCLK,
    // ===PORTB===
    TEST_PIN_OUT,
    USART3_TX,
    USART3_RX,
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
      Gpio(Gpio::USART2_TX,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_2, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
           GPIO_PinSource2),
      Gpio(Gpio::USART2_RX,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_3, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING},
           GPIO_PinSource3),
      Gpio(Gpio::USART1_TX,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_9, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
           GPIO_PinSource9),
      Gpio(Gpio::USART1_RX,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_10, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING},
           GPIO_PinSource10),
      Gpio(Gpio::SWDIO,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_13, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
           GPIO_PinSource13),
      Gpio(Gpio::SWCLK,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_14, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
           GPIO_PinSource14),
      // ===================PORTB=================
      Gpio(Gpio::TEST_PIN_OUT,
           GPIOB_BASE,
           GPIO_InitTypeDef {GPIO_Pin_0, GPIO_Speed_50MHz, GPIO_Mode_Out_PP}),
      Gpio(Gpio::USART3_TX,
           GPIOB_BASE,
           GPIO_InitTypeDef {GPIO_Pin_10, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
           GPIO_PinSource10),
      Gpio(Gpio::USART3_RX,
           GPIOB_BASE,
           GPIO_InitTypeDef {GPIO_Pin_11, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING},
           GPIO_PinSource11),
      // ===================PORTC=================
      // ===================PORTD=================
      // ===================PORTE=================
      // ===================PORTF=================
      // ===================PORTG=================
      Gpio(Gpio::__ENUM__SIZE,
           GPIOD_BASE,
           GPIO_InitTypeDef { GPIO_Pin_14, GPIO_Speed_50MHz, GPIO_Mode_AF_PP })
  } };

static constexpr const std::array<const uint32_t, 1> RemappingContainer =
{{
     GPIO_Remap_SWJ_JTAGDisable
 }};

#endif /* SOURCES_PMD_GPIO_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_GPIO_CONFIG_DESCRIPTION_H_ */
