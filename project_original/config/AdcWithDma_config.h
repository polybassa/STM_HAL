// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_ADCWITHDMA_CONFIG_CONTAINER_H_
#define SOURCES_PMD_ADCWITHDMA_CONFIG_CONTAINER_H_

static constexpr const size_t NUMBER_OF_ADC_WITH_DMA = 2;

static constexpr const std::array<const AdcWithDma, NUMBER_OF_ADC_WITH_DMA> Container =
{ {
      AdcWithDma(hal::Factory<hal::Adc::Channel>::getForDMA<hal::Adc::Channel::Description::IA_FB>(),
                 ADC_DMAMode_Circular,
                 hal::Factory<hal::Dma>::get<hal::Dma::Description::ADC3_DMA>()),
      AdcWithDma(hal::Factory<hal::Adc::Channel>::getForDMA<hal::Adc::Channel::Description::I_TOTAL_FB>(),
                 ADC_DMAMode_Circular,
                 hal::Factory<hal::Dma>::get<hal::Dma::Description::ADC4_DMA>())
  } };
#endif /* SOURCES_PMD_ADCWITHDMA_CONFIG_CONTAINER_H_ */
