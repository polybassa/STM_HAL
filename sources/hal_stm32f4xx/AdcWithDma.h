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

#ifndef SOURCES_PMD_ADCWITHDMA_H_
#define SOURCES_PMD_ADCWITHDMA_H_

#include <cstdint>
#include <array>
#include "Dma.h"
#include "AdcChannel.h"
#include "Semaphore.h"
#include "hal_Factory.h"
#include "stm32f4xx_syscfg.h"

namespace hal
{
struct AdcWithDma {
    AdcWithDma() = delete;
    AdcWithDma(const AdcWithDma&) = delete;
    AdcWithDma(AdcWithDma &&) = default;
    AdcWithDma& operator=(const AdcWithDma&) = delete;
    AdcWithDma& operator=(AdcWithDma &&) = delete;

    template<size_t n>
    void startConversion(const std::array<uint16_t, n>& data, os::Semaphore* dataAvailable = nullptr) const;
    template<size_t n>
    void startConversion(const std::array<uint16_t, n>& data, std::function<void(void)> callBack) const;
    void stopConversion(void) const;

    void startConversion(uint16_t const* const data, const size_t length, os::Semaphore* dataAvailableSemaphore) const;
    void startConversion(uint16_t const* const data, const size_t length, std::function<void(void)> callBack) const;

    float getVoltage(const uint16_t) const;
    float getVoltage(const float) const;

    uint32_t getAdcSampleTime(void) const;

private:
    constexpr AdcWithDma(const Adc::Channel& adcChannel,
                         const uint32_t      adcDmaMode,
                         const Dma&          dma) :
        mAdcChannel(adcChannel), mAdcDmaMode(adcDmaMode),
        mDma(dma) {}

    const Adc::Channel& mAdcChannel;
    const uint32_t mAdcDmaMode;
    const Dma& mDma;

    void initialize(void) const;
    friend class Factory<AdcWithDma>;
};

template<size_t n>
void AdcWithDma::startConversion(const std::array<uint16_t, n>& data, os::Semaphore* dataAvailable) const
{
    startConversion(data.data(), data.size(), dataAvailable);
}

template<size_t n>
void AdcWithDma::startConversion(const std::array<uint16_t, n>& data, std::function<void(void)> callBack) const
{
    startConversion(data.data(), data.size(), callBack);
}

template<>
class Factory<AdcWithDma>
{
#include "AdcWithDma_config.h"

    Factory(void)
    {
        SYSCFG_DMAChannelRemapConfig(SYSCFG_DMARemap_ADC2ADC4, ENABLE);

        for (const auto& obj : Container) {
            obj.initialize();
        }
    }

    template<hal::Adc::Channel::Description desc>
    static constexpr const AdcWithDma& _get(const size_t i)
    {
        return Container[i].mAdcChannel.mDescription == desc ? (Container[i]) : _get<desc>(i + 1);
    }

public:

    template<hal::Adc::Channel::Description desc>
    static constexpr const AdcWithDma& get(void)
    {
        return _get<desc>(0);
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_ADCWITHDMA_H_ */
