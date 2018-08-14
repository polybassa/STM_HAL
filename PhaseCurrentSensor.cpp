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

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::AdcWithDma;
using hal::Factory;
using hal::HalfBridge;
using hal::PhaseCurrentSensor;
using hal::Tim;

void PhaseCurrentSensor::setPulsWidthForTriggerPerMill(uint32_t value) const
{
    if (value > HalfBridge::MAXIMAL_PWM_IN_MILL) {
        value = HalfBridge::MAXIMAL_PWM_IN_MILL;
    }

    if (value < HalfBridge::MINIMAL_PWM_IN_MILL) {
        value = HalfBridge::MINIMAL_PWM_IN_MILL;
    }

    float scale = static_cast<float>(mHBridge.mTim.getPeriode()) /
                  static_cast<float>(HalfBridge::MAXIMAL_PWM_IN_MILL);

    static const uint32_t sampleTime = 2 << mAdcWithDma.getAdcSampleTime();

    // TODO Magic Numbers refactoring

    value = static_cast<uint32_t>(static_cast<float>(value) * scale) * (0.45); // + value / 5000.0);
    value += sampleTime;

    TIM_SetCompare4(mHBridge.mTim.getBasePointer(),
                    static_cast<uint32_t>(std::max(
                                                   static_cast<int32_t>(value + HalfBridge::DEFAULT_DEADTIME),
                                                   static_cast<int32_t>(1))));
}

void PhaseCurrentSensor::setNumberOfMeasurementsForPhaseCurrentValue(uint32_t value) const
{
    if (value > MAX_NUMBER_OF_MEASUREMENTS) {
        value = MAX_NUMBER_OF_MEASUREMENTS;
    }
    if (value == 0) {
        value = 1;
    }
    mNumberOfMeasurementsForPhaseCurrentValue = value;
    mAdcWithDma.stopConversion();
    mAdcWithDma.startConversion(
                                MeasurementValueBuffer[mDescription].data(), mNumberOfMeasurementsForPhaseCurrentValue,
                                [&] {this->updateCurrentValue();
                                });
}

size_t PhaseCurrentSensor::getNumberOfMeasurementsForPhaseCurrentValue(void) const
{
    return mNumberOfMeasurementsForPhaseCurrentValue;
}

void PhaseCurrentSensor::updateCurrentValue(void) const
{
    auto& array = MeasurementValueBuffer[mDescription];

    for (size_t i = 0; i < mNumberOfMeasurementsForPhaseCurrentValue; i++) {
        mPhaseCurrentValue -= mPhaseCurrentValue / FILTERWIDTH;
        mPhaseCurrentValue += static_cast<float>(array[i]) / FILTERWIDTH;
    }

    if (mValueAvailableSemaphore) {
        mValueAvailableSemaphore->giveFromISR();
    }
}

void PhaseCurrentSensor::registerValueAvailableSemaphore(os::Semaphore* valueAvailable) const
{
    mValueAvailableSemaphore = valueAvailable;
}

void PhaseCurrentSensor::unregisterValueAvailableSemaphore(void) const
{
    mValueAvailableSemaphore = nullptr;
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
}

float PhaseCurrentSensor::getPhaseCurrent(void) const
{
    static constexpr const float A_PER_DIGITS = 1.0 / 53.8;

    return static_cast<float>(mOffsetValue - mPhaseCurrentValue) *
           A_PER_DIGITS;
}

float PhaseCurrentSensor::getCurrentVoltage(void) const
{
    return mAdcWithDma.getVoltage(mPhaseCurrentValue);
}

void PhaseCurrentSensor::initialize(void) const
{
    TIM_OC4Init(mHBridge.mTim.getBasePointer(), &mAdcTrgoConfiguration);
    TIM_OC4PreloadConfig(mHBridge.mTim.getBasePointer(), TIM_OCPreload_Enable);

    TIM_SelectMasterSlaveMode(mHBridge.mTim.getBasePointer(), TIM_MasterSlaveMode_Enable);

    /* Channel 4 output compare signal is connected to TRGO */
    TIM_SelectOutputTrigger(mHBridge.mTim.getBasePointer(), (uint16_t)TIM_TRGOSource_OC4Ref);
    TIM_SelectOutputTrigger2(mHBridge.mTim.getBasePointer(), (uint16_t)TIM_TRGOSource_OC4Ref);
    setPulsWidthForTriggerPerMill(1);
}

constexpr const std::array<const PhaseCurrentSensor,
                           PhaseCurrentSensor::Description::__ENUM__SIZE> Factory<PhaseCurrentSensor>::Container;
std::array<std::array<uint16_t,
                      PhaseCurrentSensor::MAX_NUMBER_OF_MEASUREMENTS>,
           PhaseCurrentSensor::Description::__ENUM__SIZE> PhaseCurrentSensor::MeasurementValueBuffer;
