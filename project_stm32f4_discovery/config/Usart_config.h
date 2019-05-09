// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_USART_INTERRUPTS_H_
#define SOURCES_PMD_USART_INTERRUPTS_H_

#endif /* SOURCES_PMD_USART_INTERRUPTS_H_ */

#ifndef SOURCES_PMD_USART_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_USART_CONFIG_DESCRIPTION_H_

enum Description {
    DEBUG_IF,
    DISCO_DEMO_COM,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_USART_CONFIG_CONTAINER_H_
#define SOURCES_PMD_USART_CONFIG_CONTAINER_H_

static constexpr const std::array<const Usart, Usart::__ENUM__SIZE> Container =
{ {
      // no USART Level inversion possible on STM32F4
      Usart(Usart::DEBUG_IF,
            USART3_BASE,
            USART_InitTypeDef { 115200, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No,
                                USART_Mode_Rx | USART_Mode_Tx, USART_HardwareFlowControl_None },
            Factory<Nvic>::get<Nvic::DEBUG_IF>()),
      Usart(Usart::DISCO_DEMO_COM,
            UART4_BASE,
            USART_InitTypeDef { 115200, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No,
                                USART_Mode_Rx | USART_Mode_Tx, USART_HardwareFlowControl_None},
            Factory<Nvic>::get<Nvic::DISCO_DEMO_COM>()),
  }};

static constexpr const std::array<const uint32_t, hal::Usart::__ENUM__SIZE> Clocks =
{ {
      RCC_APB1Periph_USART3,
      RCC_APB1Periph_UART4,
  }};

#endif /* SOURCES_PMD_USART_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_USART_CONFIG_DESCRIPTION_H_ */
