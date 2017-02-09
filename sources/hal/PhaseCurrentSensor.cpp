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

#include "PhaseCurrentSensor.h"
#include "trace.h"
#include <algorithm>
#include "RealTimeDebugInterface.h"

extern dev::RealTimeDebugInterface* g_RTTerminal;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::AdcWithDma;
using hal::Factory;
using hal::HalfBridge;
using hal::PhaseCurrentSensor;
using hal::Tim;

void PhaseCurrentSensor::setPulsWidthForTriggerPerMill(uint32_t value) const
{
    static constexpr const uint32_t maxValue = 1000;
    static constexpr const uint32_t minValue = 25;
    if (value > maxValue) {
        value = maxValue;
    }

    if (value < minValue) {
        value = minValue;
    }

    prevVal = value;

    float scale = static_cast<float>(mHBridge.mTim.getPeriode()) /
                  static_cast<float>(maxValue);

    value = static_cast<uint32_t>(static_cast<float>(value) * scale) * (0.7 + value / 5000.0);

    const uint32_t sampleTime = 2 << mAdcWithDma.mAdcChannel.mSampleTime;
    TIM_SetCompare4(mHBridge.mTim.getBasePointer(),
                    static_cast<uint32_t>(std::max(
                                                   static_cast<int32_t>(value + HalfBridge::DEFAULT_DEADTIME),
                                                   static_cast<int32_t>(1))));
}

void PhaseCurrentSensor::updateCurrentValue(void) const
{
    auto& array = MeasurementValueBuffer[mDescription];

    for (size_t i = 0; i < PhaseCurrentSensor::NUMBER_OF_MEASUREMENTS_FOR_AVG; i++) {
        mPhaseCurrentValue -= mPhaseCurrentValue / mFilterWidth;
        mPhaseCurrentValue += static_cast<float>(array[i]) / mFilterWidth;
    }
}

void PhaseCurrentSensor::registerValueAvailableSemaphore(os::Semaphore* valueAvailable) const
{
    mAdcWithDma.mDma.registerInterruptSemaphore(valueAvailable, hal::Dma::InterruptSource::TC);
}

void PhaseCurrentSensor::registerValueAvailableSemaphore(os::Semaphore* valueAvailable, bool doubleSpeed) const
{
    if (doubleSpeed) {
        mAdcWithDma.mDma.registerInterruptSemaphore(valueAvailable, hal::Dma::InterruptSource::HT);
    } else {
        mAdcWithDma.mDma.registerInterruptSemaphore(valueAvailable, hal::Dma::InterruptSource::TC);
    }
}

void PhaseCurrentSensor::unregisterValueAvailableSemaphore(void) const
{
    mAdcWithDma.mDma.unregisterInterruptSemaphore(hal::Dma::InterruptSource::TC);
}

void PhaseCurrentSensor::unregisterValueAvailableSemaphore(bool doubleSpeed) const
{
    if (doubleSpeed) {
        mAdcWithDma.mDma.unregisterInterruptSemaphore(hal::Dma::InterruptSource::HT);
    } else {
        mAdcWithDma.mDma.unregisterInterruptSemaphore(hal::Dma::InterruptSource::TC);
    }
}

void PhaseCurrentSensor::enable(void) const
{
    mAdcWithDma.startConversion(MeasurementValueBuffer[mDescription], [&] {this->updateCurrentValue();
                                });
}

void PhaseCurrentSensor::disable(void) const
{
    mAdcWithDma.stopConversion();
}

void PhaseCurrentSensor::reset(void) const
{
    mPhaseCurrentValue = 2 * mOffsetValue - mPhaseCurrentValue;
}

void PhaseCurrentSensor::calibrate(void) const
{
    os::ThisTask::sleep(std::chrono::milliseconds(250));
    mOffsetValue = mPhaseCurrentValue;
    mOffsetVoltage = mAdcWithDma.getVoltage(mPhaseCurrentValue);
}

float PhaseCurrentSensor::getPhaseCurrent(void) const
{
    static constexpr const float SHUNT_CONDUCTANCE = 1 / SHUNT_RESISTANCE;
    float powerFactor = 1;
    static const constexpr float fact = 2;

    if (prevVal < mPowerFactorBorder) {
        powerFactor = static_cast<float>(prevVal) / (mPowerFactorBorder * fact) + 1 / fact;
    }

    return (mOffsetVoltage -
            mAdcWithDma.getVoltage(mPhaseCurrentValue)) *
           SHUNT_CONDUCTANCE / MEASUREMENT_GAIN * powerFactor;
}

float PhaseCurrentSensor::getCurrentVoltage(void) const
{
    return mAdcWithDma.getVoltage(mPhaseCurrentValue);
}

void PhaseCurrentSensor::setOffsetVoltage(float offsetVoltage) const
{
    if ((offsetVoltage > 0) && (offsetVoltage < 3.3)) {
        mOffsetVoltage = offsetVoltage;
    }
}

void PhaseCurrentSensor::initialize(void) const
{
    TIM_OC4Init(mHBridge.mTim.getBasePointer(), &mAdcTrgoConfiguration);
    TIM_OC4PreloadConfig(mHBridge.mTim.getBasePointer(), TIM_OCPreload_Enable);

    TIM_SelectMasterSlaveMode(mHBridge.mTim.getBasePointer(), TIM_MasterSlaveMode_Enable);

    /* Channel 4 output compare signal is connected to TRGO */
    TIM_SelectOutputTrigger(mHBridge.mTim.getBasePointer(), (uint16_t)TIM_TRGOSource_OC4Ref);

    setPulsWidthForTriggerPerMill(1);
}

constexpr const std::array<const PhaseCurrentSensor,
                           PhaseCurrentSensor::Description::__ENUM__SIZE> Factory<PhaseCurrentSensor>::Container;
std::array<std::array<uint16_t,
                      PhaseCurrentSensor::NUMBER_OF_MEASUREMENTS_FOR_AVG>,
           PhaseCurrentSensor::Description::__ENUM__SIZE> PhaseCurrentSensor::MeasurementValueBuffer;
