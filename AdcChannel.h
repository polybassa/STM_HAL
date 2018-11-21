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

#ifndef SOURCES_PMD_ADC_CHANNEL_H_
#define SOURCES_PMD_ADC_CHANNEL_H_

#include <cstdint>
#include <limits>
#include <array>
#include <chrono>
#include "hal_Factory.h"
#include "Adc.h"

namespace hal
{
struct Adc::Channel {
#include "Adc_Channel_config.h"

    Channel() = delete;
    Channel(const Channel&) = delete;
    Channel(Channel &&) = default;
    Channel& operator=(const Channel&) = delete;
    Channel& operator=(Channel &&) = delete;

    uint32_t getValue(void) const;
    uint32_t getCalibrationValue(void) const;
    float getVoltage(void) const;
    float getVoltage(const uint16_t) const;
    float getVoltage(const float) const;

    const enum Description mDescription;

private:
    constexpr Channel(
                      const enum Description          desc,
                      const Adc&                      baseAdc,
                      const uint8_t                   channel,
                      const uint8_t                   sampleTime,
                      const std::chrono::milliseconds cacheTime = std::chrono::milliseconds(1),
                      const float                     maxVoltage = 3.3,
                      const uint8_t                   rank = 1) :
        mDescription(desc),
        mBaseAdc(baseAdc),
        mChannel(channel), mSampleTime(sampleTime),
        mCacheTimeInTicks(cacheTime.count() / portTICK_PERIOD_MS),
        mMaxVoltage(maxVoltage), mRank(rank) {}

    const Adc& mBaseAdc;
    const uint8_t mChannel;
    const uint8_t mSampleTime;
    const uint32_t mCacheTimeInTicks;
    const float mMaxVoltage;
    const uint8_t mRank;
    mutable uint32_t mLastUpdateTicks = 0;
    mutable uint32_t mCacheValue = 0;

    void initialize(void) const;

    void startConversion(void) const;

    friend struct Adc;
    friend struct AdcWithDma;
    friend class Factory<Adc::Channel>;
    friend struct PhaseCurrentSensor;
};

template<>
class Factory<Adc::Channel>
{
#include "Adc_Channel_config.h"

    Factory(void)
    {
        for (const auto& adc : Container) {
            adc.initialize();
        }
    }

public:
    template<enum Adc::Channel::Description index>
    static constexpr const Adc::Channel& getForDMA(void)
    {
        static_assert(IS_ADC_CHANNEL(Container[index].mChannel), "Invalid Parameter");
        static_assert(IS_ADC_SAMPLE_TIME(Container[index].mSampleTime), "Invalid Parameter");
        static_assert(IS_ADC_CHANNEL(Container[index].mRank), "Invalid Parameter");
        static_assert(Container[index].mBaseAdc.mDescription < Adc::Description::__ENUM__SIZE,
                      "Invalid Parameter");
        static_assert(Container[index].mDescription < Adc::Channel::Description::__ENUM__SIZE,
                      "Invalid Parameter");
        static_assert(index != Adc::Channel::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        static_assert(portTICK_PERIOD_MS == 1, "Wrong OS tick rate configuration. 1 ms per tick required");

        static_assert(Container[index].mBaseAdc.mConfiguration.ADC_ContinuousConvMode == 0x00 /*TODO changed ADC_ContinuousConvMode_Disable*/,
                      "Invalid Parameter of base adc. This Channel can't be used directly");
        static_assert(
                      Container[index].mBaseAdc.mConfiguration.ADC_ExternalTrigConvEdge !=
                      ADC_ExternalTrigConvEdge_None,
                      "Invalid Parameter of base adc. This Channel can't be used for DMA");

        return Container[index];
    }

    template<enum Adc::Channel::Description index>
    static constexpr const Adc::Channel& get(void)
    {
        static_assert(IS_ADC_CHANNEL(Container[index].mChannel), "Invalid Parameter");
        static_assert(IS_ADC_SAMPLE_TIME(Container[index].mSampleTime), "Invalid Parameter");
        static_assert(IS_ADC_CHANNEL(Container[index].mRank), "Invalid Parameter");
        static_assert(Container[index].mBaseAdc.mDescription < Adc::Description::__ENUM__SIZE,
                      "Invalid Parameter");
        static_assert(Container[index].mDescription < Adc::Channel::Description::__ENUM__SIZE,
                      "Invalid Parameter");
        static_assert(index != Adc::Channel::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        static_assert(portTICK_PERIOD_MS == 1, "Wrong OS tick rate configuration. 1 ms per tick required");

        static_assert(Container[index].mBaseAdc.mConfiguration.ADC_ContinuousConvMode == DISABLE,
                      "Invalid Parameter of base adc. This Channel can't be used directly");
        static_assert(
                      Container[index].mBaseAdc.mConfiguration.ADC_ExternalTrigConvEdge ==
                      ADC_ExternalTrigConvEdge_None,
                      "Invalid Parameter of base adc. This Channel can't be used directly");

        return Container[index];
    }
    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_ADC_CHANNEL_H_ */
