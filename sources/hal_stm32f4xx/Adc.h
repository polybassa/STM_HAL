/* Copyright (C) 2018  Nils Weiss and Henning Mende
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
#include "Nvic.h"

namespace hal
{
struct Adc {
#include "Adc_config.h"
    struct Channel;

    Adc() = delete;
    Adc(const Adc&) = delete;
    Adc(Adc&&) = default;
    Adc& operator=(const Adc&) = delete;
    Adc& operator=(Adc&&) = delete;

    const enum Description mDescription;

private:
    constexpr Adc(const enum Description&      desc,
                  const uint32_t&              peripherie,
                  const ADC_InitTypeDef&       conf,
                  const ADC_CommonInitTypeDef& commonConf,
                  const uint16_t               resolution = 4095,
                  const Nvic&                  nvic = Factory<Nvic>::getByIrqChannel<IRQn::ADC_IRQn>()) :
        mDescription(desc), mPeripherie(peripherie),
        mConfiguration(conf),
        mCommonConfiguration(commonConf),
        mResolution(resolution),
        mNvic(nvic) {}

    const uint32_t mPeripherie;
    const ADC_InitTypeDef mConfiguration;
    const ADC_CommonInitTypeDef mCommonConfiguration;
    const uint16_t mResolution;
    const Nvic& mNvic;

    ADC_TypeDef* getBasePointer(void) const;
    void initialize(void) const;
    uint16_t getValue(const Adc::Channel&) const;
    float getVoltage(const Adc::Channel&) const;
    void startConversion(const Adc::Channel&) const;

public:
    static void handleInterrupt(void);

    static std::array<os::Semaphore, Description::__ENUM__SIZE> ConversionCompleteSemaphores;
    static std::array<os::Mutex, Description::__ENUM__SIZE> ConverterAvailableMutex;
    static constexpr const uint8_t TWO_CONVERSION_SAMPLE_DELAY = 0;
    static constexpr const uint32_t INTERRUPT_PRIORITY = 0xa;

    friend class Factory<Adc>;
    friend class Factory<Adc::Channel>;
    friend struct AdcWithDma;
};

template<>
class Factory<Adc>
{
#include "Adc_config.h"

    Factory(void)
    {
        // INFO: To speedup ADC Conversion, choose a smaller divider e.g. 6

        // enable only the adc clocks that are really used
        for (const hal::Adc& adc : Container) {
            switch (adc.mPeripherie) {
            case ADC1_BASE:
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
                break;

            case ADC2_BASE:
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
                break;

            case ADC3_BASE:
                RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
                break;

            default:
                break;
            }
        }

        ADC_DeInit();

        for (const auto& adc : Container) {
            adc.initialize();
        }
    }
public:

    template<enum Adc::Description index>
    static constexpr const Adc& get(void)
    {
        static_assert(index < Container.size(), "Index out of bounds ");
        static_assert(Container[index].mDescription < Adc::Description::__ENUM__SIZE, "Invalid Parameter");

        static_assert((Container[index].mPeripherie == ADC1_BASE) ||
                      (Container[index].mPeripherie == ADC2_BASE) ||
                      (Container[index].mPeripherie == ADC3_BASE), "Invalid Peripheries ");
        static_assert(IS_ADC_MODE(Container[index].mCommonConfiguration.ADC_Mode), "Invalid Parameter");
        static_assert(IS_ADC_PRESCALER(Container[index].mCommonConfiguration.ADC_Prescaler), "Invalid Parameter");
        static_assert(IS_ADC_DMA_ACCESS_MODE(
                                             Container[index].mCommonConfiguration.ADC_DMAAccessMode),
                      "Invalid Parameter");
        static_assert(IS_ADC_SAMPLING_DELAY(
                                            Container[index].mCommonConfiguration.ADC_TwoSamplingDelay),
                      "Invalid Parameter");

        static_assert(IS_ADC_RESOLUTION(Container[index].mConfiguration.ADC_Resolution), "Invalid Parameter");
        static_assert(IS_FUNCTIONAL_STATE(Container[index].mConfiguration.ADC_ScanConvMode), "Invalid Parameter");
        static_assert(IS_FUNCTIONAL_STATE(Container[index].mConfiguration.ADC_ContinuousConvMode), "Invalid Parameter");
        static_assert(IS_ADC_EXT_TRIG_EDGE(
                                           Container[index].mConfiguration.ADC_ExternalTrigConvEdge),
                      "Invalid Parameter");
        static_assert(IS_ADC_EXT_TRIG(Container[index].mConfiguration.ADC_ExternalTrigConv), "Invalid Parameter");
        static_assert(IS_ADC_DATA_ALIGN(Container[index].mConfiguration.ADC_DataAlign), "Invalid Parameter");
        static_assert(Container[index].mConfiguration.ADC_NbrOfConversion >= 1, "Invalid Parameter");
        static_assert(Container[index].mConfiguration.ADC_NbrOfConversion <= 16, "Invalid Parameter");

        static_assert(index != Adc::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_ADC_H_ */
