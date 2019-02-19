// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

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
    SECCO_PWR,
    LED,
    USART3_TX,
    USART3_RX,
    SPI2_NSS,
    SPI2_SCK,
    SPI2_MISO,
    SPI2_MOSI,
    // ===PORTC===
    MODEM_RESET,
    MODEM_POWER,
    MODEM_SUPPLY,
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
      Gpio(Gpio::SECCO_PWR,
           GPIOB_BASE,
           GPIO_InitTypeDef {GPIO_Pin_0, GPIO_Speed_50MHz, GPIO_Mode_Out_PP}),
      Gpio(Gpio::LED,
           GPIOB_BASE,
           GPIO_InitTypeDef {GPIO_Pin_8, GPIO_Speed_50MHz, GPIO_Mode_Out_PP}),
      Gpio(Gpio::USART3_TX,
           GPIOB_BASE,
           GPIO_InitTypeDef {GPIO_Pin_10, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
           GPIO_PinSource10),
      Gpio(Gpio::USART3_RX,
           GPIOB_BASE,
           GPIO_InitTypeDef {GPIO_Pin_11, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING},
           GPIO_PinSource11),
      Gpio(Gpio::SPI2_NSS,
           GPIOB_BASE,
           GPIO_InitTypeDef {GPIO_Pin_12, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
           GPIO_PinSource12),
      Gpio(Gpio::SPI2_SCK,
           GPIOB_BASE,
           GPIO_InitTypeDef {GPIO_Pin_13, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
           GPIO_PinSource13),
      Gpio(Gpio::SPI2_MISO,
           GPIOB_BASE,
           GPIO_InitTypeDef {GPIO_Pin_14, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
           GPIO_PinSource14),
      Gpio(Gpio::SPI2_MOSI,
           GPIOB_BASE,
           GPIO_InitTypeDef {GPIO_Pin_15, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
           GPIO_PinSource15),

      // ===================PORTC=================
      Gpio(Gpio::MODEM_RESET,
           GPIOC_BASE,
           GPIO_InitTypeDef {GPIO_Pin_7, GPIO_Speed_50MHz, GPIO_Mode_Out_PP}),
      Gpio(Gpio::MODEM_POWER,
           GPIOC_BASE,
           GPIO_InitTypeDef {GPIO_Pin_8, GPIO_Speed_50MHz, GPIO_Mode_Out_PP}),
      Gpio(Gpio::MODEM_SUPPLY,
           GPIOC_BASE,
           GPIO_InitTypeDef {GPIO_Pin_9, GPIO_Speed_50MHz, GPIO_Mode_Out_PP}),
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
