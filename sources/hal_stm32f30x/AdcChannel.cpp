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
