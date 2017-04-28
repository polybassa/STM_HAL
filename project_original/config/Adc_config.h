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

#ifndef SOURCES_PMD_ADC_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_ADC_CONFIG_DESCRIPTION_H_

enum Description {
    PMD_ADC1,
    PMD_ADC2,
    PMD_ADC3,
    PMD_ADC4,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_ADC_CONFIG_CONTAINER_H_
#define SOURCES_PMD_ADC_CONFIG_CONTAINER_H_

static constexpr std::array<const Adc, Adc::Description::__ENUM__SIZE> Container =
{ {
      Adc(Adc::PMD_ADC1,
          ADC1_BASE,
          {ADC_ContinuousConvMode_Disable, ADC_Resolution_12b, ADC_ExternalTrigConvEvent_0,
           ADC_ExternalTrigEventEdge_None, ADC_DataAlign_Right, ADC_OverrunMode_Disable, ADC_AutoInjec_Disable, 1},
          {ADC_Mode_Independent, ADC_Clock_AsynClkMode, ADC_DMAAccessMode_Disabled, ADC_DMAMode_OneShot,
           Adc::TWO_CONVERSION_SAMPLE_DELAY},
          IRQn::ADC1_2_IRQn),
      Adc(Adc::PMD_ADC2,
          ADC2_BASE,
          {ADC_ContinuousConvMode_Disable, ADC_Resolution_12b, ADC_ExternalTrigConvEvent_0,
           ADC_ExternalTrigEventEdge_None, ADC_DataAlign_Right, ADC_OverrunMode_Disable, ADC_AutoInjec_Disable, 1},
          {ADC_Mode_Independent, ADC_Clock_AsynClkMode, ADC_DMAAccessMode_Disabled, ADC_DMAMode_OneShot,
           Adc::TWO_CONVERSION_SAMPLE_DELAY},
          IRQn::ADC1_2_IRQn),
      Adc(Adc::PMD_ADC3,
          ADC3_BASE,
          {ADC_ContinuousConvMode_Disable, ADC_Resolution_12b, ADC_ExternalTrigConvEvent_9,
           ADC_ExternalTrigEventEdge_RisingEdge, ADC_DataAlign_Right, ADC_OverrunMode_Disable, ADC_AutoInjec_Disable, 1},
          {ADC_Mode_Independent, ADC_Clock_SynClkModeDiv1, ADC_DMAAccessMode_Disabled, ADC_DMAMode_OneShot,
           Adc::TWO_CONVERSION_SAMPLE_DELAY},
          IRQn::ADC3_IRQn),
      Adc(Adc::PMD_ADC4,
          ADC4_BASE,
          {ADC_ContinuousConvMode_Disable, ADC_Resolution_12b, ADC_ExternalTrigConvEvent_9,
           ADC_ExternalTrigEventEdge_RisingEdge, ADC_DataAlign_Right, ADC_OverrunMode_Disable, ADC_AutoInjec_Disable, 1},
          {ADC_Mode_Independent, ADC_Clock_SynClkModeDiv1, ADC_DMAAccessMode_Disabled, ADC_DMAMode_OneShot,
           Adc::TWO_CONVERSION_SAMPLE_DELAY},
          IRQn::ADC4_IRQn)
  } };

#endif /* SOURCES_PMD_ADC_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_ADC_CONFIG_DESCRIPTION_H_ */
