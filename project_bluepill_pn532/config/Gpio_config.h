// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_GPIO_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_GPIO_CONFIG_DESCRIPTION_H_

enum Description {
    // ===PORTA===
    SCA_INPUT,
    TRIGGER1,
    SPI1_NSS,
    SPI1_SCK,
    SPI1_MISO,
    SPI1_MOSI,
    USB1,
    USB2,
    SWDIO,
    SWCLK,
    // ===PORTB===
    TRIGGER2,
    // ===PORTC===
    LED,
    // ===PORTD===
    USB_DISCON,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_GPIO_CONFIG_CONTAINER_H_
#define SOURCES_PMD_GPIO_CONFIG_CONTAINER_H_

static constexpr const std::array<const Gpio, Gpio::__ENUM__SIZE + 1> Container =
{ {
      // ===================PORTA=================
      Gpio(Gpio::SCA_INPUT,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_0, GPIO_Speed_50MHz, GPIO_Mode_AIN},
           GPIO_PinSource0),
      Gpio(Gpio::TRIGGER1,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_3, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING},
           GPIO_PinSource3),
      Gpio(Gpio::SPI1_NSS,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_4, GPIO_Speed_50MHz, GPIO_Mode_Out_OD},
           GPIO_PinSource4),
      Gpio(Gpio::SPI1_SCK,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_5, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
           GPIO_PinSource5),
      Gpio(Gpio::SPI1_MISO,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_6, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING},
           GPIO_PinSource6),
      Gpio(Gpio::SPI1_MOSI,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_7, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
           GPIO_PinSource7),
      Gpio(Gpio::USB1,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_11, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
           GPIO_PinSource11),
      Gpio(Gpio::USB2,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_12, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
           GPIO_PinSource12),
      Gpio(Gpio::SWDIO,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_13, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
           GPIO_PinSource13),
      Gpio(Gpio::SWCLK,
           GPIOA_BASE,
           GPIO_InitTypeDef {GPIO_Pin_14, GPIO_Speed_50MHz, GPIO_Mode_AF_PP},
           GPIO_PinSource14),
      // ===================PORTB=================
      Gpio(Gpio::TRIGGER2,
           GPIOB_BASE,
           GPIO_InitTypeDef {GPIO_Pin_3, GPIO_Speed_50MHz, GPIO_Mode_IN_FLOATING},
           GPIO_PinSource3),
      // ===================PORTC=================
      Gpio(Gpio::LED,
           GPIOC_BASE,
           GPIO_InitTypeDef {GPIO_Pin_13, GPIO_Speed_50MHz, GPIO_Mode_Out_PP}),
      // ===================PORTD=================
      Gpio(Gpio::USB_DISCON,
           GPIOD_BASE,
           GPIO_InitTypeDef {GPIO_Pin_9, GPIO_Speed_50MHz, GPIO_Mode_Out_OD}),
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
