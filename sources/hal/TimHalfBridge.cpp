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

#include "TimHalfBridge.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Factory;
using hal::HalfBridge;
using hal::Tim;

void HalfBridge::setPulsWidthPerMill(uint32_t value) const
{
    static constexpr float maxValue = 1000;
    if (value > maxValue) {
        value = maxValue;
    }
    mPulsWidth = value;

    static const float scale = mTim.mConfiguration.TIM_Period / maxValue;
    value = value * scale;

    TIM_SetCompare1(mTim.getBasePointer(), value);
    TIM_SetCompare2(mTim.getBasePointer(), value);
    TIM_SetCompare3(mTim.getBasePointer(), value);
}

uint32_t HalfBridge::getPulsWidthPerMill(void) const
{
    return mPulsWidth;
}

void HalfBridge::setBridgeA(const bool highSide, const bool lowSide) const
{
    setOutputForChannel(TIM_Channel_1, highSide, lowSide);
}

void HalfBridge::setBridgeB(const bool highSide, const bool lowSide) const
{
    setOutputForChannel(TIM_Channel_2, highSide, lowSide);
}

void HalfBridge::setBridgeC(const bool highSide, const bool lowSide) const
{
    setOutputForChannel(TIM_Channel_3, highSide, lowSide);
}

void HalfBridge::setBridge(const std::array<const bool, 6>& states) const
{
    setBridgeA(states[0], states[1]);
    setBridgeB(states[2], states[3]);
    setBridgeC(states[4], states[5]);
}

void HalfBridge::setOutputForChannel(const uint16_t channel, const bool highState, const bool lowState) const
{
    if (!IS_TIM_CHANNEL(channel)) {
        // ERROR
        return;
    }
    if (highState) {
        // PWM at low side FET of bridge U
        // active freewheeling at high side FET of bridge U
        // if low side FET is in PWM off mode then the hide side FET
        // is ON for active freewheeling. This mode needs correct definition
        // of dead time otherwise we have shoot-through problems
        TIM_SelectOCxM(mTim.getBasePointer(), channel, TIM_OCMode_PWM1);
        TIM_CCxCmd(mTim.getBasePointer(), channel, TIM_CCx_Enable);
        // active freewheeling
        TIM_CCxNCmd(mTim.getBasePointer(), channel, TIM_CCxN_Enable);
        // uncomment to disable active freewheeling
        //TIM_CCxNCmd(mTim.getBasePointer(), channel, TIM_CCxN_Disable);
    } else {
        // Low side FET: OFF
        TIM_CCxCmd(mTim.getBasePointer(), channel, TIM_CCx_Disable);
        if (lowState) {
            // High side FET: ON
            TIM_SelectOCxM(
                           mTim.getBasePointer(), channel, TIM_ForcedAction_Active);
            TIM_CCxNCmd(mTim.getBasePointer(), channel, TIM_CCxN_Enable);
        } else {
            // High side FET: OFF
            TIM_SelectOCxM(
                           mTim.getBasePointer(), channel, TIM_ForcedAction_InActive);
            TIM_CCxNCmd(mTim.getBasePointer(), channel,
                        TIM_CCxN_Enable);
        }
    }
}

void HalfBridge::initialize(void) const
{
    /* Channel 1, 2, 3 are set to PWM mode */
    TIM_OC1Init(mTim.getBasePointer(), &mOcConfiguration);
    TIM_OC2Init(mTim.getBasePointer(), &mOcConfiguration);
    TIM_OC3Init(mTim.getBasePointer(), &mOcConfiguration);

    TIM_OC1PreloadConfig(mTim.getBasePointer(), TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(mTim.getBasePointer(), TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(mTim.getBasePointer(), TIM_OCPreload_Enable);

    TIM_BDTRConfig(mTim.getBasePointer(), &mBdtrConfiguration);

    TIM_CCPreloadControl(mTim.getBasePointer(), ENABLE);
}

constexpr const std::array<const HalfBridge, HalfBridge::Description::__ENUM__SIZE> Factory<HalfBridge>::Container;
