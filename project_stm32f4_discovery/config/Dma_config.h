// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_DMA_INTERRUPTS_H_
#define SOURCES_PMD_DMA_INTERRUPTS_H_

#endif /* SOURCES_PMD_DMA_INTERRUPTS_H_ */

#ifndef SOURCES_PMD_DMA_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_DMA_CONFIG_DESCRIPTION_H_

enum Description {
    MEMORY,
    TEST_ADC,
    DISCO_DEMO_COM_RX,
    DISCO_DEMO_COM_TX,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_DMA_CONFIG_CONTAINER_H_
#define SOURCES_PMD_DMA_CONFIG_CONTAINER_H_

static constexpr const std::array<const Dma, Dma::__ENUM__SIZE + 1> Container =
{ {
      Dma(Dma::MEMORY,
          DMA2_Stream3_BASE,
          DMA_InitTypeDef {0, 0, 0, 0, 1, 0}),
      Dma(Dma::TEST_ADC,
          DMA2_Stream0_BASE,
          DMA_InitTypeDef { DMA_Channel_0, reinterpret_cast<uint32_t>(&(ADC1->DR)), 0,
                            DMA_DIR_PeripheralToMemory, 1, DMA_PeripheralInc_Disable,
                            DMA_MemoryInc_Enable, DMA_PeripheralDataSize_HalfWord,
                            DMA_MemoryDataSize_HalfWord, DMA_Mode_Normal,
                            DMA_Priority_High, DMA_FIFOMode_Disable, DMA_FIFOThreshold_3QuartersFull,
                            DMA_MemoryBurst_Single, DMA_PeripheralBurst_Single},
          DMA_IT_TC, &Factory<Nvic>::get<Nvic::ADC1_DMA_INT>()),
      Dma(Dma::DISCO_DEMO_COM_RX,
          DMA1_Stream2_BASE,
          DMA_InitTypeDef { DMA_Channel_4, reinterpret_cast<uint32_t>(&(UART4->DR)), 0,
                            DMA_DIR_PeripheralToMemory, 15, DMA_PeripheralInc_Disable,
                            DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte,
                            DMA_MemoryDataSize_Byte, DMA_Mode_Normal,
                            DMA_Priority_High, DMA_FIFOMode_Disable, DMA_FIFOThreshold_3QuartersFull,
                            DMA_MemoryBurst_Single, DMA_PeripheralBurst_Single},
          DMA_IT_TC, &Factory<Nvic>::get<Nvic::USART4_DMA_RX>()),
      Dma(Dma::DISCO_DEMO_COM_TX,
          DMA1_Stream4_BASE,
          DMA_InitTypeDef { DMA_Channel_4, reinterpret_cast<uint32_t>(&(UART4->DR)), 0,
                            DMA_DIR_MemoryToPeripheral, 15, DMA_PeripheralInc_Disable,
                            DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte,
                            DMA_MemoryDataSize_Byte, DMA_Mode_Normal,
                            DMA_Priority_High, DMA_FIFOMode_Disable, DMA_FIFOThreshold_3QuartersFull,
                            DMA_MemoryBurst_Single, DMA_PeripheralBurst_Single},
          DMA_IT_TC, &Factory<Nvic>::get<Nvic::USART4_DMA_TX>()),
      Dma(Dma::__ENUM__SIZE,
          0,
          DMA_InitTypeDef { })
  } };

#endif /* SOURCES_PMD_DMA_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_DMA_CONFIG_DESCRIPTION_H_ */
