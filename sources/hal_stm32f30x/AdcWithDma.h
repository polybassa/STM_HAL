// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_ADCWITHDMA_H_
#define SOURCES_PMD_ADCWITHDMA_H_

#include <cstdint>
#include <array>
#include "Dma.h"
#include "AdcChannel.h"
#include "Semaphore.h"
#include "hal_Factory.h"
#include "stm32f30x_syscfg.h"

namespace hal
{
struct AdcWithDma {
    AdcWithDma() = delete;
    AdcWithDma(const AdcWithDma&) = delete;
    AdcWithDma(AdcWithDma&&) = default;
    AdcWithDma& operator=(const AdcWithDma&) = delete;
    AdcWithDma& operator=(AdcWithDma&&) = delete;

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
