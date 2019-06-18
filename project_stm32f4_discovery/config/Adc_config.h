// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_ADC_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_ADC_CONFIG_DESCRIPTION_H_

enum Description {
    HAL_ADC1,
    HAL_ADC2,
    HAL_ADC3,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_ADC_CONFIG_CONTAINER_H_
#define SOURCES_PMD_ADC_CONFIG_CONTAINER_H_

static constexpr std::array<const Adc, Adc::Description::__ENUM__SIZE> Container =
{ {
      Adc(Adc::HAL_ADC1,
          ADC1_BASE,
          ADC_InitTypeDef {ADC_Resolution_10b, DISABLE, DISABLE, ADC_ExternalTrigConvEdge_None, 0,
                           ADC_DataAlign_Right,
                           1},
          ADC_CommonInitTypeDef {ADC_Mode_Independent, ADC_Prescaler_Div2, ADC_DMAAccessMode_1,
                                 ADC_TwoSamplingDelay_5Cycles},
          1023 /*4095*/ /*Bits resolution*/),
      Adc(Adc::HAL_ADC2,
          ADC2_BASE,
          ADC_InitTypeDef {ADC_Resolution_10b, DISABLE, DISABLE, ADC_ExternalTrigConvEdge_None, 0, ADC_DataAlign_Right,
                           1},
          ADC_CommonInitTypeDef {ADC_Mode_Independent, ADC_Prescaler_Div2, ADC_DMAAccessMode_Disabled,
                                 ADC_TwoSamplingDelay_5Cycles},
          1023 /*Bits resolution*/),
      Adc(Adc::HAL_ADC3,
          ADC3_BASE,
          ADC_InitTypeDef {ADC_Resolution_10b, DISABLE, DISABLE, ADC_ExternalTrigConvEdge_None, 0, ADC_DataAlign_Right,
                           1},
          ADC_CommonInitTypeDef {ADC_Mode_Independent, ADC_Prescaler_Div2, ADC_DMAAccessMode_Disabled,
                                 ADC_TwoSamplingDelay_5Cycles},
          1023 /*Bits resolution*/),
  } };

#endif /* SOURCES_PMD_ADC_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_ADC_CONFIG_DESCRIPTION_H_ */
