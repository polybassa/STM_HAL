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
#include "TimSensorBldc.h"
#include "trace.h"
#include "DRV8302MotorController.h"
#include "RealTimeDebugInterface.h"

extern dev::RealTimeDebugInterface* g_RTTerminal;
extern app::DRV8302MotorController* g_motorCtrl;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using dev::SensorBLDC;
using hal::HalfBridge;
using hal::HallDecoder;
using hal::Tim;

float SensorBLDC::getCurrentRPS(void) const
{
    const float maximumRPS = 70;

    const float rpsHallDecoder = mHallDecoder.getCurrentRPS();
    const float rpsHallMeter1 = mHallMeter1.getCurrentRPS();
    const float rpsHallMeter2 = mHallMeter2.getCurrentRPS();

    const float rpsMean = (rpsHallDecoder + rpsHallMeter1 + rpsHallMeter2) / 3.0;

    const float factorHighSpeedResolution = rpsMean / 100;
    const float factorLowSpeedResolution = (maximumRPS - rpsMean) / 100;

    const float rps =
        (rpsHallDecoder * factorLowSpeedResolution + rpsHallMeter1 *
         factorHighSpeedResolution +
         rpsHallMeter2 * factorHighSpeedResolution) / (factorHighSpeedResolution +
                                                       factorHighSpeedResolution +
                                                       factorLowSpeedResolution);

    if (mCurrentDirection == Direction::BACKWARD) {
        return 0.0 - rps;
    } else {
        return rps;
    }
}

float SensorBLDC::getCurrentOmega(void) const
{
#ifndef M_PI
    static constexpr float M_PI = 3.14159265358979323846f;
#endif

    return getCurrentRPS() * 2 * M_PI;
}

SensorBLDC::Direction SensorBLDC::getSetDirection(void) const
{
    return mSetDirection;
}

SensorBLDC::Direction SensorBLDC::getCurrentDirection(void) const
{
    return mCurrentDirection;
}

int32_t SensorBLDC::getPulsWidthPerMill(void) const
{
    return mHBridge.getPulsWidthPerMill();
}

void SensorBLDC::setPulsWidthInMill(int32_t value) const
{
    uint32_t absVal = std::abs(value);

    static const float maxRPS = 72000000 / 42 / mLongPWMPeriod / mPeriodSecurityOffset;

    if ((absVal < mMotorMinPWM) && (getCurrentRPS() < maxRPS)) {
        if (mHBridge.mTim.getPeriode() == mShortPWMPeriod) {
            mHBridge.mTim.setPeriode(mLongPWMPeriod);
            mPhaseCurrentSensor.registerValueAvailableSemaphore(&g_motorCtrl->mPhaseCurrentValueAvailable, true);
        }
    } else if ((absVal > mMotorReturnPWM) && (mHBridge.mTim.getPeriode() == mLongPWMPeriod)) {
        mHBridge.mTim.setPeriode(mShortPWMPeriod);
        mPhaseCurrentSensor.unregisterValueAvailableSemaphore(true);
    }

    mHBridge.setPulsWidthPerMill(absVal);
    mPhaseCurrentSensor.setPulsWidthForTriggerPerMill(absVal);
}

void SensorBLDC::setDirection(const Direction dir) const
{
    if (mSetDirection != dir) {
        mUpdateSetDirection = dir;
    }
}

size_t SensorBLDC::getNextHallPosition(const size_t position) const
{
    if (mCurrentDirection == Direction::BACKWARD) {
        static const size_t positions[] = {0, 5, 3, 1, 6, 4, 2, 0};
        return positions[position];
    } else {
        static const size_t positions[] = {0, 3, 6, 2, 5, 1, 4, 0};
        return positions[position];
    }
}

SensorBLDC::Direction SensorBLDC::getCurrentDirection(const size_t lastPosition, const size_t currentPosition) const
{
    static const size_t nextBackwardPositions[] = {0, 5, 3, 1, 6, 4, 2, 0};
    static const size_t nextForwardPositions[] = {0, 3, 6, 2, 5, 1, 4, 0};

    if (nextBackwardPositions[lastPosition] == currentPosition) {
        return Direction::BACKWARD;
    } else if (nextForwardPositions[lastPosition] == currentPosition) {
        return Direction::FORWARD;
    }

    // This should never happen
    //Trace(ZONE_ERROR, "Invalid current motor direction!!");
    g_RTTerminal->printf("ERROR IN GET DIRECTION\r\n");

    return Direction::FORWARD;
}

size_t SensorBLDC::getPreviousHallPosition(const size_t position) const
{
    if (mCurrentDirection == Direction::FORWARD) {
        static const size_t positions[] = {0, 5, 3, 1, 6, 4, 2, 0};
        return positions[position];
    } else {
        static const size_t positions[] = {0, 3, 6, 2, 5, 1, 4, 0};
        return positions[position];
    }
}

void SensorBLDC::computeDirection(void) const
{
    const uint32_t currentPosition = mHallDecoder.getCurrentHallState();

    const bool directionChanged = getPreviousHallPosition(currentPosition) != mLastHallPosition;

    if (directionChanged) {
        mCurrentDirection = getCurrentDirection(mLastHallPosition, currentPosition);
    }

    mLastHallPosition = currentPosition;
}

void SensorBLDC::enableManualCommutation(void) const
{
    mHBridge.disableTimerCommunication();
    mManualCommutationActive = true;
}

void SensorBLDC::disableManualCommutation(const size_t hallPosition) const
{
    manualCommutation(hallPosition);
    mHBridge.enableTimerCommunication();
    prepareCommutation(hallPosition);
    mManualCommutationActive = false;
}

void SensorBLDC::commutate(const size_t hallPosition) const
{
    if (mUpdateSetDirection != mSetDirection) {
        mPhaseCurrentSensor.reset();
        mSetDirection = mUpdateSetDirection;
    }

    if ((mManualCommutationActive == true) && (mSetDirection == mCurrentDirection) &&
        (std::abs(mHallDecoder.getCurrentRPS()) > 15.0))
    {
        disableManualCommutation(hallPosition);
        return;
    }

    if ((mManualCommutationActive == false) && (mSetDirection != mCurrentDirection)) {
        enableManualCommutation();
    }

    if (mManualCommutationActive == true) {
        manualCommutation(hallPosition);
    } else {
        prepareCommutation(hallPosition);
    }
}

float SensorBLDC::getPhaseCurrent(void) const
{
    return mSetDirection == Direction::FORWARD ? 0.0 -
           mPhaseCurrentSensor.getPhaseCurrent() : mPhaseCurrentSensor.getPhaseCurrent();
}

void SensorBLDC::manualCommutation(const size_t hallPosition) const
{
    /*
     * 4Q Control
     *
     *  BRAKE      Q2| ACCELERATE Q1
     *  FORWARD      | FORWARD
     *  -------------+-------------
     *  ACCELERATE Q3| BRAKE      Q4
     *  BACKWARD     | BACKWARD
     *
     */

    static const std::array<std::array<const bool, 6>, 8> BLDC_BRIDGE_STATE_FORWARD_ACCELERATE = // Motor step
    {{
         // A AN  B BN  C CN
         { 0, 0, 0, 0, 0, 0 }, // V0
         { 0, 1, 0, 0, 1, 0 }, // V3
         { 0, 0, 1, 0, 0, 1 }, // V1
         { 0, 1, 1, 0, 0, 0 }, // V2
         { 1, 0, 0, 1, 0, 0 }, // V5
         { 0, 0, 0, 1, 1, 0 }, // V4
         { 1, 0, 0, 0, 0, 1 }, // V6
         { 0, 0, 0, 0, 0, 0 } // V0
     }};

    static const std::array<std::array<const bool, 6>, 8> BLDC_BRIDGE_STATE_BACKWARD_ACCELERATE = // Motor step
    {{
         // A AN  B BN  C CN
         { 0, 0, 0, 0, 0, 0 }, // V0
         { 0, 0, 1, 0, 0, 1 }, // V1
         { 1, 0, 0, 1, 0, 0 }, // V5
         { 1, 0, 0, 0, 0, 1 }, // V6
         { 0, 1, 0, 0, 1, 0 }, // V3
         { 0, 1, 1, 0, 0, 0 }, // V2
         { 0, 0, 0, 1, 1, 0 }, // V4
         { 0, 0, 0, 0, 0, 0 } // V0
     }};

    if (mSetDirection == Direction::FORWARD) {
        mHBridge.setBridge(BLDC_BRIDGE_STATE_FORWARD_ACCELERATE[hallPosition]);
        mHBridge.triggerCommutationEvent();
    } else {
        mHBridge.setBridge(BLDC_BRIDGE_STATE_BACKWARD_ACCELERATE[hallPosition]);
        mHBridge.triggerCommutationEvent();
    }
}

void SensorBLDC::prepareCommutation(const size_t hallPosition) const
{
    /*
     * 4Q Control
     *
     *  BRAKE      Q2| ACCELERATE Q1
     *  FORWARD      | FORWARD
     *  -------------+-------------
     *  ACCELERATE Q3| BRAKE      Q4
     *  BACKWARD     | BACKWARD
     *
     */

    static const std::array<std::array<const bool, 6>, 8> BLDC_BRIDGE_STATE_ACCELERATE = // Motor step
    {{
         // A AN  B BN  C CN
         { 0, 0, 0, 0, 0, 0 }, // V0
         { 0, 1, 1, 0, 0, 0 }, // V2
         { 1, 0, 0, 0, 0, 1 }, // V6
         { 0, 0, 1, 0, 0, 1 }, // V1
         { 0, 0, 0, 1, 1, 0 }, // V4
         { 0, 1, 0, 0, 1, 0 }, // V3
         { 1, 0, 0, 1, 0, 0 }, // V5
         { 0, 0, 0, 0, 0, 0 } // V0
     }};

    mHBridge.setBridge(BLDC_BRIDGE_STATE_ACCELERATE[hallPosition]);
}

void SensorBLDC::start(void) const
{
    mHallDecoder.registerCommutationCallback([&] {
                                                 this->commutate(this->mHallDecoder.getCurrentHallState());
                                             });

    mHallDecoder.registerHallEventCheckCallback([&] {
                                                    this->computeDirection();
                                                });

    mHallDecoder.mTim.enable();
    mHallMeter1.mTim.enable();
    mHallMeter2.mTim.enable();
    mPhaseCurrentSensor.enable();

    setPulsWidthInMill(0);

    mHallDecoder.reset();
    mHallMeter1.reset();
    mHallMeter2.reset();

    mLastHallPosition = mHallDecoder.getCurrentHallState();
    enableManualCommutation();
    commutate(mLastHallPosition);

    mHBridge.enableOutput();
}

void SensorBLDC::stop(void) const
{
    mHBridge.disableOutput();
    mPhaseCurrentSensor.disable();
    mHallDecoder.mTim.disable();
    mHallMeter1.mTim.disable();
    mHallMeter2.mTim.disable();

    mHallDecoder.unregisterHallEventCheckCallback();
    mHallDecoder.unregisterCommutationCallback();
}

void SensorBLDC::checkMotor(void) const
{}

uint32_t SensorBLDC::getNumberOfPolePairs(void) const
{
    return mHallDecoder.POLE_PAIRS;
}

constexpr std::array<const dev::SensorBLDC,
                     dev::SensorBLDC::Description::__ENUM__SIZE> dev::Factory<dev::SensorBLDC>::Container;
