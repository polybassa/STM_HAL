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

#ifndef SOURCES_PMD_DMA_INTERRUPTS_H_
#define SOURCES_PMD_DMA_INTERRUPTS_H_

#define DMA1_CHANNEL1_INTERRUPT_ENABLED false
#define DMA1_CHANNEL2_INTERRUPT_ENABLED false
#define DMA1_CHANNEL3_INTERRUPT_ENABLED false
#define DMA1_CHANNEL4_INTERRUPT_ENABLED false
#define DMA1_CHANNEL5_INTERRUPT_ENABLED true
#define DMA1_CHANNEL6_INTERRUPT_ENABLED true
#define DMA1_CHANNEL7_INTERRUPT_ENABLED true
#define DMA2_CHANNEL1_INTERRUPT_ENABLED false
#define DMA2_CHANNEL2_INTERRUPT_ENABLED true
#define DMA2_CHANNEL3_INTERRUPT_ENABLED false
#define DMA2_CHANNEL4_INTERRUPT_ENABLED true
#define DMA2_CHANNEL5_INTERRUPT_ENABLED true

#endif /* SOURCES_PMD_DMA_INTERRUPTS_H_ */

#ifndef SOURCES_PMD_DMA_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_DMA_CONFIG_DESCRIPTION_H_

enum Description {
    // DMA1
    SPI2_TX,
    USART2_RX,
    USART2_TX,
    // DMA2
    SPI3_TX,
    MEMORY,
    ADC4_DMA,
    ADC3_DMA,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_DMA_CONFIG_CONTAINER_H_
#define SOURCES_PMD_DMA_CONFIG_CONTAINER_H_

static constexpr const std::array<const Dma, Dma::__ENUM__SIZE + 1> Container =
{ {
      Dma(Dma::SPI2_TX,
          DMA1_Channel5_BASE,
          DMA_InitTypeDef { SPI2_BASE + 0x0C, 0, DMA_DIR_PeripheralDST, 0, DMA_PeripheralInc_Disable,
                            DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte,
                            DMA_MemoryDataSize_Byte, DMA_Mode_Normal,
                            DMA_Priority_Low, DMA_M2M_Disable },
          DMA_IT_TC, IRQn_Type::DMA1_Channel5_IRQn),
      Dma(Dma::USART2_RX,
          DMA1_Channel6_BASE,
          DMA_InitTypeDef { USART2_BASE + 0x24, 0, DMA_DIR_PeripheralSRC, 0, DMA_PeripheralInc_Disable,
                            DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte,
                            DMA_MemoryDataSize_Byte, DMA_Mode_Circular,
                            DMA_Priority_Medium, DMA_M2M_Disable},
          DMA_IT_TC, IRQn_Type::DMA1_Channel6_IRQn),
      Dma(Dma::USART2_TX,
          DMA1_Channel7_BASE,
          DMA_InitTypeDef { USART2_BASE + 0x28, 0, DMA_DIR_PeripheralDST, 0, DMA_PeripheralInc_Disable,
                            DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte,
                            DMA_MemoryDataSize_Byte, DMA_Mode_Circular,
                            DMA_Priority_Medium, DMA_M2M_Disable},
          DMA_IT_TC, IRQn_Type::DMA1_Channel7_IRQn),
      Dma(Dma::SPI3_TX,
          DMA2_Channel2_BASE,
          DMA_InitTypeDef { SPI3_BASE + 0x0C, 0, DMA_DIR_PeripheralDST, 0, DMA_PeripheralInc_Disable,
                            DMA_MemoryInc_Enable, DMA_PeripheralDataSize_Byte,
                            DMA_MemoryDataSize_Byte, DMA_Mode_Normal,
                            DMA_Priority_Low, DMA_M2M_Disable },
          DMA_IT_TC, IRQn_Type::DMA2_Channel2_IRQn),
      Dma(Dma::MEMORY,
          DMA2_Channel3_BASE,
          DMA_InitTypeDef {}),
      Dma(Dma::ADC4_DMA,
          DMA2_Channel4_BASE,
          DMA_InitTypeDef { ADC4_BASE + 0x40, 0, DMA_DIR_PeripheralSRC, 0, DMA_PeripheralInc_Disable,
                            DMA_MemoryInc_Enable, DMA_PeripheralDataSize_HalfWord,
                            DMA_MemoryDataSize_HalfWord, DMA_Mode_Circular,
                            DMA_Priority_Medium, DMA_M2M_Disable },
          DMA_IT_TC, IRQn_Type::DMA2_Channel4_IRQn),
      Dma(Dma::ADC3_DMA,
          DMA2_Channel5_BASE,
          DMA_InitTypeDef { ADC3_BASE + 0x40, 0, DMA_DIR_PeripheralSRC, 0, DMA_PeripheralInc_Disable,
                            DMA_MemoryInc_Enable, DMA_PeripheralDataSize_HalfWord,
                            DMA_MemoryDataSize_HalfWord, DMA_Mode_Circular,
                            DMA_Priority_Medium, DMA_M2M_Disable },
          DMA_IT_TC, IRQn_Type::DMA2_Channel5_IRQn),
      Dma(Dma::__ENUM__SIZE,
          0,
          DMA_InitTypeDef { })
  } };

#endif /* SOURCES_PMD_DMA_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_DMA_CONFIG_DESCRIPTION_H_ */
