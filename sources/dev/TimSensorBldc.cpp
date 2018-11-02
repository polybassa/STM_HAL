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

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using dev::SensorBLDC;
using hal::HalfBridge;
using hal::HallDecoder;
using hal::Tim;

float SensorBLDC::getActualRPS(void) const
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

    if (mCurrentDirection == Direction::FORWARD) {
        return 0.0 - rps;
    } else {
        return rps;
    }
}

float SensorBLDC::getActualOmega(void) const
{
#ifndef M_PI
    static constexpr const float M_PI = 3.14159265358979323846f;
#endif

    return getActualRPS() * 2 * M_PI;
}

SensorBLDC::Direction SensorBLDC::getSetDirection(void) const
{
    return mSetDirection;
}

SensorBLDC::Direction SensorBLDC::getActualDirection(void) const
{
    return mCurrentDirection;
}

int32_t SensorBLDC::getActualPulsWidthPerMill(void) const
{
    return mHBridge.getPulsWidthPerMill();
}

void SensorBLDC::modifyPulsWidthPeriode(void) const
{
    //TODO make this dynamic
    // get SAMPLES = 0.4x + 12
    // get PWMFreq = 200x + 6000

    static const constexpr uint32_t MIN_PWM_FREQ = 8000;
    static const constexpr uint32_t MAX_PWM_FREQ = 24000;
    static const constexpr uint32_t MIN_SAMPLES = 16;
    static const constexpr float MIN_RPS = 10.0;
    static const constexpr uint32_t TIMER_CLOCK = 72000000;

    const float rps = std::max(std::abs(getActualRPS()), MIN_RPS);

    uint32_t samples = static_cast<uint32_t>(rps * 0.4) + 12;
    samples = std::max(MIN_SAMPLES, samples);

    uint32_t pwm_freq = static_cast<uint32_t>(rps * 200) + 6000;
    pwm_freq = std::min(MAX_PWM_FREQ, pwm_freq);
    pwm_freq = std::max(MIN_PWM_FREQ, pwm_freq);

    const uint32_t pwm_periode = TIMER_CLOCK / pwm_freq;

    mHBridge.mTim.setPeriode(pwm_periode);
    mPhaseCurrentSensor.setNumberOfMeasurementsForPhaseCurrentValue(samples);
}

void SensorBLDC::setPulsWidthInMill(int32_t value) const
{
    uint32_t absVal = std::abs(value);

    modifyPulsWidthPeriode();

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

    Trace(ZONE_ERROR, "ERROR IN GET DIRECTION\r\n");

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

void SensorBLDC::calibrate(void) const
{
    mHBridge.setupOutputsForCalibration();
    mPhaseCurrentSensor.setPulsWidthForTriggerPerMill(20);
    mPhaseCurrentSensor.calibrate();
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

float SensorBLDC::getActualPhaseCurrent(void) const
{
    return mSetDirection == Direction::FORWARD ? 0.0 -
           (mPhaseCurrentSensor.getPhaseCurrent() * 0.75 -
            0.0079) : mPhaseCurrentSensor.getPhaseCurrent()* 0.75 - 0.0079;
}

float SensorBLDC::getActualTorqueInNewtonMeter(void) const
{
    return getActualPhaseCurrent() * mMotorConstant;
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
{
    // TODO: Add some checks for overcurrent or something like that
}

uint32_t SensorBLDC::getNumberOfPolePairs(void) const
{
    return mHallDecoder.POLE_PAIRS;
}

constexpr std::array<const dev::SensorBLDC,
                     dev::SensorBLDC::Description::__ENUM__SIZE> dev::Factory<dev::SensorBLDC>::Container;
