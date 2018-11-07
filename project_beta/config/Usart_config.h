// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

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
                                USART_Mode_Rx | USART_Mode_Tx, USART_HardwareFlowControl_None},
            true),
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
