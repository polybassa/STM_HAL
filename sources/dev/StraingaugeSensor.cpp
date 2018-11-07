// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */
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
