// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "TimPwm.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Factory;
using hal::Pwm;
using hal::Tim;

void Pwm::setPulsWidthInMill(uint32_t value) const
{
    static const uint32_t maxValue = 1000;
    if (value > maxValue) {
        value = maxValue;
    }
    static const float scale = mTim.getPeriode() / maxValue;
    value = value * scale;

    switch (mChannel) {
    case CHANNEL1:
        TIM_SetCompare1(mTim.getBasePointer(), value);
        break;

    case CHANNEL2:
        TIM_SetCompare2(mTim.getBasePointer(), value);
        break;

    case CHANNEL3:
        TIM_SetCompare3(mTim.getBasePointer(), value);
        break;

    case CHANNEL4:
        TIM_SetCompare4(mTim.getBasePointer(), value);
        break;
    }
}

void Pwm::initialize(void) const
{
    switch (mChannel) {
    case CHANNEL1:
        TIM_OC1Init(mTim.getBasePointer(), &mOcConfiguration);
        TIM_OC1PreloadConfig(mTim.getBasePointer(), TIM_OCPreload_Enable);
        break;

    case CHANNEL2:
        TIM_OC2Init(mTim.getBasePointer(), &mOcConfiguration);
        TIM_OC2PreloadConfig(mTim.getBasePointer(), TIM_OCPreload_Enable);
        break;

    case CHANNEL3:
        TIM_OC3Init(mTim.getBasePointer(), &mOcConfiguration);
        TIM_OC3PreloadConfig(mTim.getBasePointer(), TIM_OCPreload_Enable);
        break;

    case CHANNEL4:
        TIM_OC4Init(mTim.getBasePointer(), &mOcConfiguration);
        TIM_OC4PreloadConfig(mTim.getBasePointer(), TIM_OCPreload_Enable);
        break;
    }

    mTim.enable();
}

constexpr const std::array<const Pwm, Pwm::Description::__ENUM__SIZE> Factory<Pwm>::Container;
