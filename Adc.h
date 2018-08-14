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

#ifndef SOURCES_PMD_ADC_H_
#define SOURCES_PMD_ADC_H_

#include <cstdint>
#include <limits>
#include <array>
#include <chrono>
#include "stm32f4xx_adc.h"
#include "stm32f4xx_rcc.h"
#include "Mutex.h"
#include "Semaphore.h"
#include "hal_Factory.h"

extern "C" void ADC1_2_IRQHandler(void);
extern "C" void ADC3_IRQHandler(void);
extern "C" void ADC4_IRQHandler(void);

namespace hal
{
struct Adc {
#include "Adc_config.h"
    struct Channel;

    Adc() = delete;
    Adc(const Adc&) = delete;
    Adc(Adc &&) = default;
    Adc& operator=(const Adc&) = delete;
    Adc& operator=(Adc &&) = delete;

    uint32_t getCalibrationValue(void) const;

    const enum Description mDescription;

private:
    constexpr Adc(const enum Description&      desc,
                  const uint32_t&              peripherie,
                  const ADC_InitTypeDef&       conf,
                  const ADC_CommonInitTypeDef& commonConf,
                  const enum IRQn              irqn,
                  const uint8_t                resolutionBits = 12) :
        mDescription(desc), mPeripherie(peripherie),
        mConfiguration(conf),
        mCommonConfiguration(commonConf),
        mIRQn(irqn), mResolutionBits(resolutionBits) {}

    const uint32_t mPeripherie;
    const ADC_InitTypeDef mConfiguration;
    const ADC_CommonInitTypeDef mCommonConfiguration;
    const enum IRQn mIRQn;
    const uint8_t mResolutionBits;

    ADC_TypeDef* getBasePointer(void) const;
    void initialize(void) const;
    void calibrate(void) const;
    uint32_t getValue(const Adc::Channel&) const;
    float getVoltage(const Adc::Channel&) const;
    void startConversion(const Adc::Channel&) const;
    void stopConversion(void) const;

    static std::array<uint32_t, Description::__ENUM__SIZE> CalibrationValues;
    static std::array<os::Semaphore, Description::__ENUM__SIZE> ConversionCompleteSemaphores;
    static std::array<os::Mutex, Description::__ENUM__SIZE> ConverterAvailableMutex;
    static constexpr uint8_t TWO_CONVERSION_SAMPLE_DELAY = 0;
    static constexpr uint32_t INTERRUPT_PRIORITY = 0xa;

    friend class Factory<Adc>;
    friend class Factory<Adc::Channel>;
    friend struct AdcWithDma;
    friend void ::ADC1_2_IRQHandler(void);
    friend void ::ADC3_IRQHandler(void);
    friend void ::ADC4_IRQHandler(void);
};

template<>
class Factory<Adc>
{
#include "Adc_config.h"

    Factory(void)
    {
        RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div1); // 72MHz
        RCC_ADCCLKConfig(RCC_ADC34PLLCLK_Div1);
        // INFO: To speedup ADC Conversion, choose a smaller divider e.g. 6

        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC34, ENABLE);

        ADC_DeInit(ADC1);
        ADC_DeInit(ADC2);
        ADC_DeInit(ADC3);
        ADC_DeInit(ADC4);

        for (const auto& adc : Container) {
            adc.calibrate();
        }
        for (const auto& adc : Container) {
            adc.initialize();
        }
    }
public:

    template<enum Adc::Description index>
    static constexpr const Adc& get(void)
    {
        static_assert(index < Container.size(), "Index out of bounds ");
        static_assert(IS_ADC_ALL_PERIPH_BASE(Container[index].mPeripherie), "Invalid Peripheries ");
        static_assert(Container[index].mDescription < Adc::Description::__ENUM__SIZE, "Invalid Parameter");
        static_assert(IS_ADC_AUTOINJECMODE(Container[index].mConfiguration.ADC_AutoInjMode), "Invalid Parameter");
        static_assert(IS_ADC_CONVMODE(Container[index].mConfiguration.ADC_ContinuousConvMode), "Invalid Parameter");
        static_assert(IS_ADC_DATA_ALIGN(Container[index].mConfiguration.ADC_DataAlign), "Invalid Parameter");
        static_assert(IS_ADC_EXT_TRIG(Container[index].mConfiguration.ADC_ExternalTrigConvEvent), "Invalid Parameter");
        static_assert(IS_EXTERNALTRIG_EDGE(
                                           Container[index].mConfiguration.ADC_ExternalTrigEventEdge),
                      "Invalid Parameter");
        static_assert(IS_ADC_CHANNEL(Container[index].mConfiguration.ADC_NbrOfRegChannel), "Invalid Parameter");
        static_assert(IS_ADC_OVRUNMODE(Container[index].mConfiguration.ADC_OverrunMode), "Invalid Parameter");
        static_assert(IS_ADC_RESOLUTION(Container[index].mConfiguration.ADC_Resolution), "Invalid Parameter");
        static_assert(IS_ADC_CLOCKMODE(Container[index].mCommonConfiguration.ADC_Clock), "Invalid Parameter");
        static_assert(IS_ADC_DMA_ACCESS_MODE(
                                             Container[index].mCommonConfiguration.ADC_DMAAccessMode),
                      "Invalid Parameter");
        static_assert(IS_ADC_DMA_MODE(Container[index].mCommonConfiguration.ADC_DMAMode), "Invalid Parameter");
        static_assert(IS_ADC_MODE(Container[index].mCommonConfiguration.ADC_Mode), "Invalid Parameter");
        static_assert(Container[index].mIRQn & (IRQn::ADC1_2_IRQn | IRQn::ADC3_IRQn | IRQn::ADC4_IRQn),
                      "Invalid Parameter");

        static_assert(index != Adc::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_ADC_H_ */
