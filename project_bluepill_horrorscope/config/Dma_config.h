// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_DMA_INTERRUPTS_H_
#define SOURCES_PMD_DMA_INTERRUPTS_H_

#define DMA1_CHANNEL1_INTERRUPT_ENABLED true
#define DMA1_CHANNEL2_INTERRUPT_ENABLED false
#define DMA1_CHANNEL3_INTERRUPT_ENABLED false
#define DMA1_CHANNEL4_INTERRUPT_ENABLED false
#define DMA1_CHANNEL5_INTERRUPT_ENABLED false
#define DMA1_CHANNEL6_INTERRUPT_ENABLED false
#define DMA1_CHANNEL7_INTERRUPT_ENABLED false
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
    ADC,
    // DMA2
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_DMA_CONFIG_CONTAINER_H_
#define SOURCES_PMD_DMA_CONFIG_CONTAINER_H_

static constexpr const std::array<const Dma, Dma::__ENUM__SIZE + 1> Container =
{ {
      Dma(Dma::ADC,
          DMA1_Channel1_BASE,
          DMA_InitTypeDef { reinterpret_cast<uint32_t>(ADC1_BASE + 0x4C), 1, DMA_DIR_PeripheralSRC, 1,
                            DMA_PeripheralInc_Disable,
                            DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Word,
                            DMA_MemoryDataSize_Word, DMA_Mode_Normal,
                            DMA_Priority_VeryHigh, DMA_M2M_Disable},
          DMA_IT_TC, IRQn_Type::DMA1_Channel1_IRQn),
      Dma(Dma::__ENUM__SIZE,
          0,
          DMA_InitTypeDef { })
  } };

#endif /* SOURCES_PMD_DMA_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_DMA_CONFIG_DESCRIPTION_H_ */
