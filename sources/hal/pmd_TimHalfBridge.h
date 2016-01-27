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

#ifndef SOURCES_PMD_HALF_BRIDGE_H_
#define SOURCES_PMD_HALF_BRIDGE_H_

#include <cstdint>
#include <array>
#include "pmd_hal_Factory.h"
#include "pmd_dev_Factory.h"
#include "pmd_Tim.h"

namespace dev
{
struct SensorBLDC;
}

namespace hal
{
struct HalfBridge {
#include "pmd_TimHalfBridge_config.h"

    HalfBridge() = delete;
    HalfBridge(const HalfBridge&) = delete;
    HalfBridge(HalfBridge&&) = default;
    HalfBridge& operator=(const HalfBridge&) = delete;
    HalfBridge& operator=(HalfBridge&&) = delete;

    void setPulsWidthPerMill(uint32_t) const;
    uint32_t getPulsWidthPerMill(void) const;

    void setBridgeA(const bool highState, const bool lowState) const;
    void setBridgeB(const bool highState, const bool lowState) const;
    void setBridgeC(const bool highState, const bool lowState) const;

    void setBridge(const std::array<const bool, 6>& states) const;

private:
    constexpr HalfBridge(const enum Description&    desc,
                         const Tim&                 timer,
                         const TIM_OCInitTypeDef&   ocConf,
                         const TIM_BDTRInitTypeDef& bdtrConf) :
        mDescription(desc), mTim(timer), mOcConfiguration(ocConf), mBdtrConfiguration(bdtrConf) {}

    const enum Description mDescription;
    const Tim& mTim;
    const TIM_OCInitTypeDef mOcConfiguration;
    const TIM_BDTRInitTypeDef mBdtrConfiguration;

    mutable size_t mPulsWidth = 0;

    void initialize(void) const;
    void setOutputForChannel(const uint16_t channel, const bool highState, const bool lowState) const;

    friend class Factory<HalfBridge>;
    friend class dev::Factory<dev::SensorBLDC>;
    friend struct dev::SensorBLDC;
};

template<>
class Factory<HalfBridge> {
#include "pmd_TimHalfBridge_config.h"

    Factory(void)
    {
        for (const auto& obj : Container) {
            if (obj.mDescription != HalfBridge::__ENUM__SIZE) {
                obj.initialize();
            }
        }
    }
public:

    template<enum HalfBridge::Description index>
    static constexpr const HalfBridge& get(void)
    {
        static_assert(IS_TIM_OC_MODE(Container[index].mOcConfiguration.TIM_OCMode), "Invalid Parameter ");
        static_assert(IS_TIM_OUTPUT_STATE(Container[index].mOcConfiguration.TIM_OutputState), "Invalid Parameter ");
        static_assert(IS_TIM_OUTPUTN_STATE(Container[index].mOcConfiguration.TIM_OutputNState), "Invalid Parameter ");
        static_assert(IS_TIM_OC_POLARITY(Container[index].mOcConfiguration.TIM_OCPolarity), "Invalid Parameter ");
        static_assert(IS_TIM_OCN_POLARITY(Container[index].mOcConfiguration.TIM_OCNPolarity), "Invalid Parameter ");
        static_assert(IS_TIM_OCIDLE_STATE(Container[index].mOcConfiguration.TIM_OCIdleState), "Invalid Parameter ");
        static_assert(IS_TIM_OCNIDLE_STATE(Container[index].mOcConfiguration.TIM_OCNIdleState), "Invalid Parameter ");

        static_assert(IS_TIM_OSSR_STATE(Container[index].mBdtrConfiguration.TIM_OSSRState), "Invalid Parameter ");
        static_assert(IS_TIM_OSSI_STATE(Container[index].mBdtrConfiguration.TIM_OSSIState), "Invalid Parameter ");
        static_assert(IS_TIM_LOCK_LEVEL(Container[index].mBdtrConfiguration.TIM_LOCKLevel), "Invalid Parameter ");
        static_assert(IS_TIM_BREAK_STATE(Container[index].mBdtrConfiguration.TIM_Break), "Invalid Parameter ");
        static_assert(IS_TIM_BREAK_POLARITY(Container[index].mBdtrConfiguration.TIM_BreakPolarity),
                      "Invalid Parameter ");
        static_assert(IS_TIM_AUTOMATIC_OUTPUT_STATE(
                          Container[index].mBdtrConfiguration.TIM_AutomaticOutput), "Invalid Parameter ");

        static_assert(Container[index].mTim.mDescription != Tim::Description::__ENUM__SIZE, "Invalid Tim Object");
        static_assert(index != HalfBridge::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_HALF_BRIDGE_H_ */
