// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_ADC_H_
#define SOURCES_PMD_ADC_H_

#include <cstdint>
#include <limits>
#include <array>
#include <chrono>
#include "stm32f10x_adc.h"
#include "stm32f10x_rcc.h"
#include "hal_Factory.h"

namespace hal
{
struct AdcChannel {
    ADC_TypeDef* ADCx;
    uint8_t ADC_Channel;
    uint8_t Rank;
    uint8_t ADC_SampleTime;
};

struct Adc {
#include "Adc_config.h"

    Adc() = delete;
    Adc(const Adc&) = delete;
    Adc(Adc&&) = default;
    Adc& operator=(const Adc&) = delete;
    Adc& operator=(Adc&&) = delete;

    const enum Description mDescription;

    void startConversion(void) const;

private:
    constexpr Adc(const enum Description& desc,
                  const uint32_t&         peripherie,
                  const ADC_InitTypeDef&  conf,
                  const bool              dmaSupport) :
        mDescription(desc), mPeripherie(peripherie),
        mConfiguration(conf), mDmaSupport(dmaSupport) {}

    const uint32_t mPeripherie;
    const ADC_InitTypeDef mConfiguration;
    const bool mDmaSupport;

    void initialize(void) const;
    void calibrate(void) const;

    friend class Factory<Adc>;
};

template<>
class Factory<Adc>
{
#include "Adc_config.h"

    Factory(void)
    {
        RCC_ADCCLKConfig(RCC_PCLK2_Div4);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);

        ADC_DeInit(ADC1);
        ADC_DeInit(ADC2);

        for (const auto& adc : Container) {
            adc.initialize();
        }
        for (const auto& adc : Container) {
            adc.calibrate();
        }
        for (const auto& channel : ChannelContainer) {
            ADC_RegularChannelConfig(channel.ADCx, channel.ADC_Channel, channel.Rank, channel.ADC_SampleTime);
        }
    }
public:

    template<enum Adc::Description index>
    static constexpr const Adc& get(void)
    {
        static_assert(index < Container.size(), "Index out of bounds ");
        static_assert(IS_ADC_ALL_PERIPH_BASE(Container[index].mPeripherie), "Invalid Peripheries ");
        static_assert(Container[index].mDescription < Adc::Description::__ENUM__SIZE, "Invalid Parameter");
        static_assert(IS_ADC_DATA_ALIGN(Container[index].mConfiguration.ADC_DataAlign), "Invalid Parameter");
        static_assert(IS_ADC_EXT_TRIG(Container[index].mConfiguration.ADC_ExternalTrigConv), "Invalid Parameter");

        static_assert(IS_ADC_MODE(Container[index].mConfiguration.ADC_Mode), "Invalid Parameter");

        static_assert(index != Adc::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_ADC_H_ */
