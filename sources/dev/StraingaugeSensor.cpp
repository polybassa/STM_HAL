/* Copyright (C) 2015  Matthias Birnthaler
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
#include <algorithm>
#include <numeric>
#include "StraingaugeSensor.h"
#include "trace.h"
#include <cmath>

static const int __attribute__((unused)) g_DebugZones = 0; // ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using dev::StraingaugeSensor;

void StraingaugeSensor::resetStraingaugeSensor(void) const
{
    mFirstFillCount = 1;
    mCurrentValueFilterIndex = 0;
    mLastFilterValue = 0.0;
    mFilterValues.fill(0);

    mGlobalMean = 0.0;
    mMean = 0.0;
    mMeanold = 0.0;
    mCurrentRange = 0;

    mFlow = 0.0;
    mTrail = 0.0;
    mChange = 0.0;
}

float StraingaugeSensor::getRawStrain(void) const
{
    const auto RawStrain = mPeripherie.getVoltage();
    //  Trace(ZONE_INFO, "%f \r\n", RawStrain);
    return RawStrain;
}

void StraingaugeSensor::FilterStrain(void) const
{
    mFilterValues[mCurrentValueFilterIndex] = getRawStrain();
    mCurrentValueFilterIndex = (mCurrentValueFilterIndex + 1) % FILTERSIZE;

    mLastFilterValue = std::accumulate(mFilterValues.begin(), mFilterValues.end(), 0.0);

    if (mFirstFillCount > FILTERSIZE) {
        mLastFilterValue = mLastFilterValue / FILTERSIZE;
    } else {
        mLastFilterValue = mLastFilterValue / mFirstFillCount;
        mFirstFillCount++;
    }
}

void StraingaugeSensor::caculateMean(void) const
{
    if (mCurrentRange < mMeanRange) {
        mMean = (mMean * mCurrentRange + mLastFilterValue) / (mCurrentRange + 1);
        mCurrentRange++;
    }
    if (mCurrentRange == mMeanRange) {
        mCurrentRange = 0;
        mMeanold = mMean;
        mMean = 0;
    }

    if (mMeanold == 0) {
        mGlobalMean = mMean;
    } else {
        mGlobalMean = (mMeanold * (mMeanRange - mCurrentRange) + mMean * (mCurrentRange)) / mMeanRange;
    }
}

void StraingaugeSensor::caculateChange(void) const
{
    mFlow = mLastFilterValue - mGlobalMean;
    mChange = (mTrail - mFlow) * mChangeFactor;
    mTrail = mFlow;
}

float StraingaugeSensor::getDirection(void) const
{
    FilterStrain();
    caculateMean();
    caculateChange();
    return mChange;
}

constexpr const std::array<const StraingaugeSensor,
                           StraingaugeSensor::Description::__ENUM__SIZE> dev::Factory<StraingaugeSensor>::Container;
