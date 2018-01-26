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

#ifndef SOURCES_PMD_ADCWITHDMA_CONFIG_CONTAINER_H_
#define SOURCES_PMD_ADCWITHDMA_CONFIG_CONTAINER_H_

static constexpr const size_t NUMBER_OF_ADC_WITH_DMA = 2;

static constexpr const std::array<const AdcWithDma, NUMBER_OF_ADC_WITH_DMA> Container =
{ {
      AdcWithDma(hal::Factory<hal::Adc::Channel>::getForDMA<hal::Adc::Channel::Description::IB_FB>(),
                 ADC_DMAMode_Circular,
                 hal::Factory<hal::Dma>::get<hal::Dma::Description::ADC3_DMA>()),
      AdcWithDma(hal::Factory<hal::Adc::Channel>::getForDMA<hal::Adc::Channel::Description::I_TOTAL_FB>(),
                 ADC_DMAMode_Circular,
                 hal::Factory<hal::Dma>::get<hal::Dma::Description::ADC4_DMA>())
  } };
#endif /* SOURCES_PMD_ADCWITHDMA_CONFIG_CONTAINER_H_ */
