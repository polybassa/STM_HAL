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

#ifndef SOURCES_PMD_PWM_H_
#define SOURCES_PMD_PWM_H_

#include <cstdint>
#include <array>
#include "hal_Factory.h"
#include "Tim.h"

namespace hal
{
struct Pwm {
#include "TimPwm_config.h"

    enum Channel {
        CHANNEL1 = 0,
        CHANNEL2,
        CHANNEL3,
        CHANNEL4
    };

    Pwm() = delete;
    Pwm(const Pwm&) = delete;
    Pwm(Pwm &&) = default;
    Pwm& operator=(const Pwm&) = delete;
    Pwm& operator=(Pwm &&) = delete;

    void setPulsWidthInMill(uint32_t) const;

    /**
     * Toggle the polarity of the complementary pwm output.
     * This only effects the advanced control timers. (Tim1 and Tim8)
     */
    void togglePolarityOfComplementaryChannel(void) const;

    /**
     * Set the polarity of the complementary pwm output.
     * This only effects the advanced control timers. (Tim1 and Tim8)
     * If set to true, the xNChannel outputs a signal with the logical opposite
     * of the reference signal. Be careful if an additional dead time is configured
     * for the used timer.
     * @param high bool indicating the polarity of the n channel
     */
    void setPolarityOfComplementaryChannel(const bool& high) const;

    /**
     * returns whether the complementary channel has a high polarity.
     * @return bool flag indicating a high polarity of the complementary channel
     */
    bool getPolarityOfComplementaryChannel(void) const;

    /**
     * returns whether the main channel has a high polarity.
     * @return bool flag indicating a high polarity of the high channel
     */
    bool getPolarityOfMainChannel(void) const;

private:
    constexpr Pwm(const enum Description&  desc,
                  const Tim&               timer,
                  const enum Channel&      channel,
                  const TIM_OCInitTypeDef& ocConf) :
        mDescription(desc), mTim(timer), mChannel(channel),
        mOcConfiguration(ocConf) {}

    const enum Description mDescription;
    const Tim& mTim;
    const enum Channel mChannel;
    const TIM_OCInitTypeDef mOcConfiguration;

    void initialize(void) const;

    friend class Factory<Pwm>;
};

template<>
class Factory<Pwm>
{
#include "TimPwm_config.h"
    Factory(void)
    {
        for (const auto& obj : Container) {
            obj.initialize();
        }
    }
public:

    template<enum Pwm::Description index>
    static constexpr const Pwm& get(void)
    {
        static_assert(IS_TIM_OC_MODE(Container[index].mOcConfiguration.TIM_OCMode), "Invalid Parameter ");
        static_assert(IS_TIM_OUTPUT_STATE(Container[index].mOcConfiguration.TIM_OutputState), "Invalid Parameter ");
        static_assert(IS_TIM_OUTPUTN_STATE(Container[index].mOcConfiguration.TIM_OutputNState), "Invalid Parameter ");
        static_assert(IS_TIM_OC_POLARITY(Container[index].mOcConfiguration.TIM_OCPolarity), "Invalid Parameter ");
        static_assert(IS_TIM_OCN_POLARITY(Container[index].mOcConfiguration.TIM_OCNPolarity), "Invalid Parameter ");
        static_assert(IS_TIM_OCIDLE_STATE(Container[index].mOcConfiguration.TIM_OCIdleState), "Invalid Parameter ");
        static_assert(IS_TIM_OCNIDLE_STATE(Container[index].mOcConfiguration.TIM_OCNIdleState), "Invalid Parameter ");
        static_assert(Container[index].mOcConfiguration.TIM_Pulse >= 0x0000, "Pulse is smaller than 0x0000");
        static_assert(Container[index].mOcConfiguration.TIM_Pulse <= 0xFFFF, "Pulse is grater than 0xFFFF");

        static_assert(Container[index].mTim.mDescription != Tim::Description::__ENUM__SIZE, "Invalid Tim Object");
        static_assert(index != Pwm::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_PWM_H_ */
