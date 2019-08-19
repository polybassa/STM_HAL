// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_GPIO_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_GPIO_CONFIG_DESCRIPTION_H_

enum Description {
    // ===PORTA===
    USER_BUTTON,
    // ===PORTC===
    ADC_DEMO_PIN,
    DISCO_DEMO_COM_TX,
    DISCO_DEMO_COM_RX,
    // ===PORTD===
    DEBUG_IF_TX,
    DEBUG_IF_RX,
    LED_GREEN,
    LED_ORANGE,
    LED_RED,
    LED_BLUE,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_GPIO_CONFIG_CONTAINER_H_
#define SOURCES_PMD_GPIO_CONFIG_CONTAINER_H_

static constexpr const std::array<const Gpio, Gpio::__ENUM__SIZE + 1> Container =
{ {
      // ===================PORTA=================
      Gpio(Gpio::USER_BUTTON,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_0, GPIO_Mode_IN, GPIO_Speed_50MHz, GPIO_OType_PP, GPIO_PuPd_DOWN},
           GPIO_PinSource0),

      // ===================PORTC=================
      Gpio(Gpio::ADC_DEMO_PIN,
           GPIOC_BASE,
           GPIO_InitTypeDef {GPIO_Pin_1, GPIO_Mode_AN, GPIO_Speed_2MHz, GPIO_OType_OD, GPIO_PuPd_NOPULL}),
      Gpio(Gpio::DISCO_DEMO_COM_TX,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_10, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_UP},
           GPIO_PinSource10,
           GPIO_AF_UART4),
      Gpio(Gpio::DISCO_DEMO_COM_RX,
           GPIOC_BASE,
           GPIO_InitTypeDef { GPIO_Pin_11, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_UP},
           GPIO_PinSource11,
           GPIO_AF_UART4),

      // ===================PORTD=================
      Gpio(Gpio::DEBUG_IF_TX,
           GPIOD_BASE,
           GPIO_InitTypeDef { GPIO_Pin_8, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_UP},
           GPIO_PinSource8,
           GPIO_AF_USART3),
      Gpio(Gpio::DEBUG_IF_RX,
           GPIOD_BASE,
           GPIO_InitTypeDef { GPIO_Pin_9, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_UP},
           GPIO_PinSource9,
           GPIO_AF_USART3),
      Gpio(Gpio::LED_GREEN,
           GPIOD_BASE,
           GPIO_InitTypeDef {GPIO_Pin_12, GPIO_Mode_OUT, GPIO_Speed_25MHz, GPIO_OType_PP, GPIO_PuPd_UP}),
      Gpio(Gpio::LED_ORANGE,
           GPIOD_BASE,
           GPIO_InitTypeDef {GPIO_Pin_13, GPIO_Mode_OUT, GPIO_Speed_25MHz, GPIO_OType_PP, GPIO_PuPd_UP}),
      Gpio(Gpio::LED_RED,
           GPIOD_BASE,
           GPIO_InitTypeDef {GPIO_Pin_14, GPIO_Mode_OUT, GPIO_Speed_25MHz, GPIO_OType_PP, GPIO_PuPd_UP}),
      Gpio(Gpio::LED_BLUE,
           GPIOD_BASE,
           GPIO_InitTypeDef {GPIO_Pin_15, GPIO_Mode_OUT, GPIO_Speed_25MHz, GPIO_OType_PP, GPIO_PuPd_UP}),

      // =================== END =================
      Gpio(Gpio::__ENUM__SIZE,
           GPIOG_BASE,
           GPIO_InitTypeDef { GPIO_Pin_All, GPIO_Mode_AF, GPIO_Speed_2MHz, GPIO_OType_PP, GPIO_PuPd_UP })
  }};

#endif /* SOURCES_PMD_GPIO_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_GPIO_CONFIG_DESCRIPTION_H_ */
