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
