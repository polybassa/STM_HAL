/* Copyright (C) 2018  Nils Weiss and Henning Mende
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
#include "AdcChannel.h"
#include "os_Task.h"

using hal::Adc;
using hal::Factory;

uint16_t Adc::Channel::getValue(void) const
{
    const int now = static_cast<int>(os::Task::getTickCount());

    if (static_cast<uint32_t>(std::abs(now - static_cast<int>(mLastUpdateTicks))) >= mCacheTimeInTicks) {
        mLastUpdateTicks = now;

        mCacheValue = mBaseAdc.getValue(*this);
    }

    return mCacheValue;
}

void Adc::Channel::startConversion(void) const
{
    mBaseAdc.startConversion(*this);
}

float Adc::Channel::getVoltage(void) const
{
    return mBaseAdc.getVoltage(*this);
}

float Adc::Channel::getVoltage(const uint16_t value) const
{
    return (mMaxVoltage * value) / mBaseAdc.mResolution;
}

float Adc::Channel::getVoltage(const float value) const
{
    return (mMaxVoltage * value) / mBaseAdc.mResolution;
}

void Adc::Channel::initialize(void) const
{
    mLastUpdateTicks = os::Task::getTickCount() - 1000;
}

constexpr std::array<const Adc::Channel, Adc::Channel::__ENUM__SIZE> Factory<Adc::Channel>::Container;
