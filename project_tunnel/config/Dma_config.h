// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_DMA_INTERRUPTS_H_
#define SOURCES_PMD_DMA_INTERRUPTS_H_

#define DMA1_CHANNEL1_INTERRUPT_ENABLED false
#define DMA1_CHANNEL2_INTERRUPT_ENABLED true
#define DMA1_CHANNEL3_INTERRUPT_ENABLED false
#define DMA1_CHANNEL4_INTERRUPT_ENABLED true
#define DMA1_CHANNEL5_INTERRUPT_ENABLED false
#define DMA1_CHANNEL6_INTERRUPT_ENABLED false
#define DMA1_CHANNEL7_INTERRUPT_ENABLED true
#define DMA2_CHANNEL1_INTERRUPT_ENABLED false
#define DMA2_CHANNEL2_INTERRUPT_ENABLED false
#define DMA2_CHANNEL3_INTERRUPT_ENABLED false
#define DMA2_CHANNEL4_INTERRUPT_ENABLED false
#define DMA2_CHANNEL5_INTERRUPT_ENABLED false

#endif /* SOURCES_PMD_DMA_INTERRUPTS_H_ */

#ifndef SOURCES_PMD_DMA_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_DMA_CONFIG_DESCRIPTION_H_

enum Description {
    // DMA1
    USART3_TX,
    USART1_TX,
    USART2_TX,
    // DMA2
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_DMA_CONFIG_CONTAINER_H_
#define SOURCES_PMD_DMA_CONFIG_CONTAINER_H_

static constexpr const std::array<const Dma, Dma::__ENUM__SIZE + 1> Container =
{ {
      Dma(Dma::USART3_TX,
          DMA1_Channel2_BASE,
          DMA_InitTypeDef { USART3_BASE + 0x4, 1, DMA_DIR_PeripheralDST, 0, DMA_PeripheralInc_Disable,
                            DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte,
                            DMA_MemoryDataSize_Byte, DMA_Mode_Normal,
                            DMA_Priority_High, DMA_M2M_Disable},
          DMA_IT_TC, IRQn_Type::DMA1_Channel2_IRQn),
      Dma(Dma::USART1_TX,
          DMA1_Channel4_BASE,
          DMA_InitTypeDef { USART1_BASE + 0x4, 1, DMA_DIR_PeripheralDST, 0, DMA_PeripheralInc_Disable,
                            DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte,
                            DMA_MemoryDataSize_Byte, DMA_Mode_Normal,
                            DMA_Priority_High, DMA_M2M_Disable},
          DMA_IT_TC, IRQn_Type::DMA1_Channel4_IRQn),
      Dma(Dma::USART2_TX,
          DMA1_Channel7_BASE,
          DMA_InitTypeDef { USART2_BASE + 0x4, 1, DMA_DIR_PeripheralDST, 0, DMA_PeripheralInc_Disable,
                            DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte,
                            DMA_MemoryDataSize_Byte, DMA_Mode_Normal,
                            DMA_Priority_High, DMA_M2M_Disable},
          DMA_IT_TC, IRQn_Type::DMA1_Channel7_IRQn),
      Dma(Dma::__ENUM__SIZE,
          0,
          DMA_InitTypeDef { })
  } };

#endif /* SOURCES_PMD_DMA_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_DMA_CONFIG_DESCRIPTION_H_ */
