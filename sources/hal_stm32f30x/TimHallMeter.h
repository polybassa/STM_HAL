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

#ifndef SOURCES_PMD_HALL_METER_H_
#define SOURCES_PMD_HALL_METER_H_

#include <cstdint>
#include <array>
#include "hal_Factory.h"
#include "Tim.h"
#include <functional>

extern "C" {
void TIM2_IRQHandler(void);
void TIM8_CC_IRQHandler(void);
void TIM8_UP_IRQHandler(void);
}

namespace hal
{
struct HallMeter {
#include "TimHallMeter_config.h"

    HallMeter() = delete;
    HallMeter(const HallMeter&) = delete;
    HallMeter(HallMeter&&) = default;
    HallMeter& operator=(const HallMeter&) = delete;
    HallMeter& operator=(HallMeter&&) = delete;

    float getCurrentRPS(void) const;
    float getCurrentOmega(void) const;

    void reset(void) const;

    const enum Description mDescription;
    const Tim& mTim;

private:
    constexpr HallMeter(const enum Description&  desc,
                        const Tim&               timer,
                        const uint16_t           inputTrigger,
                        const TIM_ICInitTypeDef& ic1Conf) :
        mDescription(desc), mTim(timer),
        mInputTrigger(inputTrigger),
        mIc1Configuration(ic1Conf) {}

    void initialize(void) const;
    void interruptHandler(void) const;
    void saveTimestamp(const uint32_t) const;

    static const size_t NUMBER_OF_TIMESTAMPS = 3;

    mutable std::array<uint32_t, NUMBER_OF_TIMESTAMPS> mTimestamps = {};
    mutable size_t mTimestampPosition = 0;

    const uint16_t mInputTrigger;
    const TIM_ICInitTypeDef mIc1Configuration;

    friend class Factory<HallMeter>;
    friend void ::TIM2_IRQHandler(void);
    friend void ::TIM8_CC_IRQHandler(void);
    friend void ::TIM8_UP_IRQHandler(void);
};

template<>
class Factory<HallMeter>
{
#include "TimHallMeter_config.h"

    Factory(void)
    {
        for (const auto& obj : Container) {
            obj.initialize();
        }
    }
public:

    template<enum HallMeter::Description index>
    static constexpr const HallMeter& get(void)
    {
        static_assert(IS_TIM_CHANNEL(Container[index].mIc1Configuration.TIM_Channel), "Invalid Parameter ");
        static_assert(IS_TIM_IC_POLARITY(Container[index].mIc1Configuration.TIM_ICPolarity), "Invalid Parameter ");
        static_assert(IS_TIM_IC_SELECTION(Container[index].mIc1Configuration.TIM_ICSelection), "Invalid Parameter ");
        static_assert(IS_TIM_IC_PRESCALER(Container[index].mIc1Configuration.TIM_ICPrescaler), "Invalid Parameter ");
        static_assert(IS_TIM_IC_FILTER(Container[index].mIc1Configuration.TIM_ICFilter), "Invalid Parameter ");

        static_assert(IS_TIM_INTERNAL_TRIGGER_SELECTION(Container[index].mInputTrigger), "Invalid Parameter ");

        static_assert(Container[index].mTim.mDescription != Tim::Description::__ENUM__SIZE, "Invalid Tim Object");
        static_assert(index != HallMeter::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_HALL_METER_H_ */
