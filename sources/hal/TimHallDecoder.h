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

#ifndef SOURCES_PMD_HALL_DECODER_H_
#define SOURCES_PMD_HALL_DECODER_H_

#include <cstdint>
#include <array>
#include "hal_Factory.h"
#include "dev_Factory.h"
#include "Tim.h"
#include <functional>

extern "C" {
void TIM3_IRQHandler(void);
}

namespace hal
{
struct HallDecoder {
#include "TimHallDecoder_config.h"

    HallDecoder() = delete;
    HallDecoder(const HallDecoder&) = delete;
    HallDecoder(HallDecoder&&) = default;
    HallDecoder& operator=(const HallDecoder&) = delete;
    HallDecoder& operator=(HallDecoder&&) = delete;

    void incrementCommutationDelay(void) const;
    void decrementCommutationDelay(void) const;
    void setCommutationDelay(const uint32_t) const;
    uint32_t getCommutationDelay(void) const;
    float getCurrentRPS(void) const;
    uint32_t getCurrentHallState(void) const;
    float getCurrentOmega(void) const;

    void registerCommutationCallback(std::function<void(void)> ) const;
    void unregisterCommutationCallback(void) const;

    void registerHallEventCheckCallback(std::function<bool(void)> ) const;
    void unregisterHallEventCheckCallback(void) const;

    const enum Description mDescription;

private:
    constexpr HallDecoder(const enum Description&  desc,
                          const Tim&               timer,
                          const TIM_ICInitTypeDef& ic1Conf,
                          const TIM_OCInitTypeDef& oc2Conf,
                          const TIM_OCInitTypeDef& oc3Conf) :
        mDescription(desc), mTim(timer), mIc1Configuration(ic1Conf), mOc2Configuration(oc2Conf), mOc3Configuration(
            oc3Conf) {}

    const Tim& mTim;
    const TIM_ICInitTypeDef mIc1Configuration;
    const TIM_OCInitTypeDef mOc2Configuration;
    const TIM_OCInitTypeDef mOc3Configuration;

    void initialize(void) const;
    void interruptHandler(void) const;
    void saveTimestamp(const uint32_t) const;

    static const size_t NUMBER_OF_TIMESTAMPS = 7;

    mutable std::array<uint32_t, NUMBER_OF_TIMESTAMPS> mTimestamps = {};

    static std::array<std::function<void(void)>, Description::__ENUM__SIZE> CommutationCallbacks;
    static std::array<std::function<bool(void)>, Description::__ENUM__SIZE> HallEventCallbacks;

    friend class Factory<HallDecoder>;
    friend struct dev::SensorBLDC;
    friend void ::TIM3_IRQHandler(void);
};

template<>
class Factory<HallDecoder> {
#include "TimHallDecoder_config.h"

    Factory(void)
    {
        for (const auto& obj : Container) {
            obj.unregisterCommutationCallback();
            obj.initialize();
        }
    }
public:

    template<enum HallDecoder::Description index>
    static constexpr const HallDecoder& get(void)
    {
        static_assert(IS_TIM_OC_MODE(Container[index].mOc2Configuration.TIM_OCMode), "Invalid Parameter ");
        static_assert(IS_TIM_OUTPUT_STATE(Container[index].mOc2Configuration.TIM_OutputState), "Invalid Parameter ");
        static_assert(IS_TIM_OUTPUTN_STATE(Container[index].mOc2Configuration.TIM_OutputNState), "Invalid Parameter ");
        static_assert(IS_TIM_OC_POLARITY(Container[index].mOc2Configuration.TIM_OCPolarity), "Invalid Parameter ");
        static_assert(IS_TIM_OCN_POLARITY(Container[index].mOc2Configuration.TIM_OCNPolarity), "Invalid Parameter ");
        static_assert(IS_TIM_OCIDLE_STATE(Container[index].mOc2Configuration.TIM_OCIdleState), "Invalid Parameter ");
        static_assert(IS_TIM_OCNIDLE_STATE(Container[index].mOc2Configuration.TIM_OCNIdleState), "Invalid Parameter ");

        static_assert(IS_TIM_OC_MODE(Container[index].mOc3Configuration.TIM_OCMode), "Invalid Parameter ");
        static_assert(IS_TIM_OUTPUT_STATE(Container[index].mOc3Configuration.TIM_OutputState), "Invalid Parameter ");
        static_assert(IS_TIM_OUTPUTN_STATE(Container[index].mOc3Configuration.TIM_OutputNState), "Invalid Parameter ");
        static_assert(IS_TIM_OC_POLARITY(Container[index].mOc3Configuration.TIM_OCPolarity), "Invalid Parameter ");
        static_assert(IS_TIM_OCN_POLARITY(Container[index].mOc3Configuration.TIM_OCNPolarity), "Invalid Parameter ");
        static_assert(IS_TIM_OCIDLE_STATE(Container[index].mOc3Configuration.TIM_OCIdleState), "Invalid Parameter ");
        static_assert(IS_TIM_OCNIDLE_STATE(Container[index].mOc3Configuration.TIM_OCNIdleState), "Invalid Parameter ");

        static_assert(IS_TIM_CHANNEL(Container[index].mIc1Configuration.TIM_Channel), "Invalid Parameter ");
        static_assert(IS_TIM_IC_POLARITY(Container[index].mIc1Configuration.TIM_ICPolarity), "Invalid Parameter ");
        static_assert(IS_TIM_IC_SELECTION(Container[index].mIc1Configuration.TIM_ICSelection), "Invalid Parameter ");
        static_assert(IS_TIM_IC_PRESCALER(Container[index].mIc1Configuration.TIM_ICPrescaler), "Invalid Parameter ");
        static_assert(IS_TIM_IC_FILTER(Container[index].mIc1Configuration.TIM_ICFilter), "Invalid Parameter ");

        static_assert(Container[index].mTim.mDescription != Tim::Description::__ENUM__SIZE, "Invalid Tim Object");
        static_assert(index != HallDecoder::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_HALL_DECODER_H_ */
