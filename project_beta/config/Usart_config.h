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

#ifndef SOURCES_PMD_USART_INTERRUPTS_H_
#define SOURCES_PMD_USART_INTERRUPTS_H_

#define USART1_INTERRUPT_ENABLED true
#define USART2_INTERRUPT_ENABLED false
#define USART3_INTERRUPT_ENABLED true
#define USART4_INTERRUPT_ENABLED false
#define USART5_INTERRUPT_ENABLED false

#endif /* SOURCES_PMD_USART_INTERRUPTS_H_ */

#ifndef SOURCES_PMD_USART_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_USART_CONFIG_DESCRIPTION_H_

enum Description {
    MSCOM_IF,
    DEBUG_IF,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_USART_CONFIG_CONTAINER_H_
#define SOURCES_PMD_USART_CONFIG_CONTAINER_H_

static constexpr const std::array<const Usart, Usart::__ENUM__SIZE + 1> Container =
{ {
      Usart(Usart::MSCOM_IF,
            USART1_BASE,
            USART_InitTypeDef { 115200, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No,
                                USART_Mode_Rx | USART_Mode_Tx, USART_HardwareFlowControl_None}),
      Usart(Usart::DEBUG_IF,
            USART3_BASE,
            USART_InitTypeDef { 115200, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No,
                                USART_Mode_Rx | USART_Mode_Tx, USART_HardwareFlowControl_None }),
      Usart(Usart::__ENUM__SIZE, 0, USART_InitTypeDef { 0, 0, 0, 0, 0, 0 })
  }};

static constexpr const std::array<const uint32_t, hal::Usart::__ENUM__SIZE> Clocks =
{ {
      RCC_APB2Periph_USART1,
      RCC_APB1Periph_USART3
  }};

#endif /* SOURCES_PMD_USART_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_USART_CONFIG_DESCRIPTION_H_ */
