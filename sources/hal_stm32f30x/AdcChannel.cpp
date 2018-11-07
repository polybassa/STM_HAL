// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include <cmath>
#include "trace.h"
#include "AdcChannel.h"
#include "os_Task.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Adc;
using hal::Factory;

uint32_t Adc::Channel::getValue(void) const
{
    const int now = static_cast<int>(os::Task::getTickCount());

    if (static_cast<uint32_t>(std::abs(now - static_cast<int>(mLastUpdateTicks))) < mCacheTimeInTicks) {
        return mCacheValue;
    }
    mLastUpdateTicks = now;

    mCacheValue = mBaseAdc.getValue(*this);
    return mCacheValue;
}

void Adc::Channel::startConversion(void) const
{
    mBaseAdc.startConversion(*this);
}

void Adc::Channel::stopConversion(void) const
{
    mBaseAdc.stopConversion();
}

float Adc::Channel::getVoltage(void) const
{
    return mBaseAdc.getVoltage(*this);
}

float Adc::Channel::getVoltage(const uint16_t value) const
{
    return (mMaxVoltage * value) / std::pow(2, mBaseAdc.mResolutionBits);
}

float Adc::Channel::getVoltage(const float value) const
{
    return (mMaxVoltage * value) / std::pow(2, mBaseAdc.mResolutionBits);
}

uint32_t Adc::Channel::getCalibrationValue(void) const
{
    return mBaseAdc.getCalibrationValue();
}

void Adc::Channel::initialize(void) const
{
    mLastUpdateTicks = os::Task::getTickCount() - 1000;
}

constexpr std::array<const Adc::Channel, Adc::Channel::__ENUM__SIZE> Factory<Adc::Channel>::Container;
