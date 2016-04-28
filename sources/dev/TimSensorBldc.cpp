/* Copyright (C) 2015  Nils Weiss, Alexander Strobl
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

void SensorBLDC::incrementCommutationDelay(void) const
{
    mHallDecoder.incrementCommutationDelay();
}

void SensorBLDC::decrementCommutationDelay(void) const
{
    mHallDecoder.decrementCommutationDelay();
}

void SensorBLDC::setCommutationDelay(const uint32_t value) const
{
    mHallDecoder.setCommutationDelay(value);
}

uint32_t SensorBLDC::getCommutationDelay(void) const
{
    return mHallDecoder.getCommutationDelay();
}

float SensorBLDC::getCurrentRPS(void) const
{
    if (mDirection == Direction::BACKWARD) {
        return 0.0 - mHallDecoder.getCurrentRPS();
    } else {
        return mHallDecoder.getCurrentRPS();
    }
}

float SensorBLDC::getCurrentOmega(void) const
{
    if (mDirection == Direction::BACKWARD) {
        return 0.0 - mHallDecoder.getCurrentOmega();
    } else {
        return mHallDecoder.getCurrentOmega();
    }
}

SensorBLDC::Direction SensorBLDC::getDirection(void) const
{
    return mDirection;
}

int32_t SensorBLDC::getPulsWidthPerMill(void) const
{
    if (mDirection == Direction::BACKWARD) {
        return 0 - mHBridge.getPulsWidthPerMill();
    } else {
        return mHBridge.getPulsWidthPerMill();
    }
}

void SensorBLDC::setPulsWidthInMill(int32_t value) const
{
    mHBridge.setPulsWidthPerMill(std::abs(value));
    if (value > 0) {
        setDirection(Direction::FORWARD);
    } else {
        setDirection(Direction::BACKWARD);
    }
}

void SensorBLDC::setDirection(const Direction dir) const
{
    mDirection = dir;
}

size_t SensorBLDC::getNextHallPosition(const size_t position) const
{
    if (mDirection == Direction::BACKWARD) {
        static const size_t positions[] = {0, 5, 3, 1, 6, 4, 2, 0};
        return positions[position];
    } else {
        static const size_t positions[] = {0, 3, 6, 2, 5, 1, 4, 0};
        return positions[position];
    }
}

size_t SensorBLDC::getPreviousHallPosition(const size_t position) const
{
    if (mDirection == Direction::FORWARD) {
        static const size_t positions[] = {0, 5, 3, 1, 6, 4, 2, 0};
        return positions[position];
    } else {
        static const size_t positions[] = {0, 3, 6, 2, 5, 1, 4, 0};
        return positions[position];
    }
}

bool SensorBLDC::checkHallEvent(void) const
{
    const size_t currentPosition = mHallDecoder.getCurrentHallState();

    if ((mLastHallPosition == 0) || (currentPosition == getNextHallPosition(mLastHallPosition))) {
        mLastHallPosition = currentPosition;
        return true;
    } else if (mLastHallPosition == currentPosition) {
        return false;
    } else {
        // wrong direction
        // force right direction
        trigger();
        return false;
    }
}

void SensorBLDC::prepareCommutation(const size_t hallPosition) const
{
#ifdef MAXON_MOTOR
    static const std::array<std::array<const bool, 6>, 8> BLDC_BRIDGE_STATE_FORWARD = // Motor step
    {{
         // A AN  B BN  C CN
         { 0, 0, 0, 0, 0, 0 }, // V0
         { 0, 0, 0, 1, 1, 0 }, // V2
         { 0, 1, 1, 0, 0, 0 }, // V4
         { 0, 1, 0, 0, 1, 0 }, // V3
         { 1, 0, 0, 0, 0, 1 }, // V6
         { 1, 0, 0, 1, 0, 0 }, // V1
         { 0, 0, 1, 0, 0, 1 }, // V5
         { 0, 0, 0, 0, 0, 0 } // V0
     }};

    static const std::array<std::array<const bool, 6>, 8> BLDC_BRIDGE_STATE_BACKWARD = // Motor step
    {{
         // A AN  B BN  C CN
         { 0, 0, 0, 0, 0, 0 }, // V0
         { 0, 1, 0, 0, 1, 0 }, // V3
         { 0, 0, 1, 0, 0, 1 }, // V5
         { 0, 1, 1, 0, 0, 0 }, // V4
         { 1, 0, 0, 1, 0, 0 }, // V1
         { 0, 0, 0, 1, 1, 0 }, // V2
         { 1, 0, 0, 0, 0, 1 }, // V6
         { 0, 0, 0, 0, 0, 0 } // V0
     }};
#elif CHINA_MOTOR
    static const std::array<std::array<const bool, 6>, 8> BLDC_BRIDGE_STATE_FORWARD = // Motor step
    {{
         { 0, 0, 0, 0, 0, 0 }, // 0
         { 1, 0, 0, 1, 0, 0 }, // 1
         { 0, 1, 0, 0, 1, 0 }, // 2
         { 0, 0, 0, 1, 1, 0 }, // 3
         { 0, 0, 1, 0, 0, 1 }, // 4
         { 1, 0, 0, 0, 0, 1 }, // 5
         { 0, 1, 1, 0, 0, 0 }, // 6
         { 0, 0, 0, 0, 0, 0 }  // 0
     }};

    static const std::array<std::array<const bool, 6>, 8> BLDC_BRIDGE_STATE_BACKWARD = // Motor step
    {{
         // A AN  B BN  C CN
         { 0, 0, 0, 0, 0, 0 }, // V0
         { 0, 0, 0, 1, 1, 0 }, // V2
         { 0, 1, 1, 0, 0, 0 }, // V4
         { 0, 1, 0, 0, 1, 0 }, // V3
         { 1, 0, 0, 0, 0, 1 }, // V6
         { 1, 0, 0, 1, 0, 0 }, // V1
         { 0, 0, 1, 0, 0, 1 }, // V5
         { 0, 0, 0, 0, 0, 0 } // V0
     }};

#else
#error "NO MOTOR DEFINED"
#endif
    if (mDirection == Direction::FORWARD) {
        mHBridge.setBridge(BLDC_BRIDGE_STATE_FORWARD[hallPosition]);
    } else {
        mHBridge.setBridge(BLDC_BRIDGE_STATE_BACKWARD[hallPosition]);
    }
}

void SensorBLDC::trigger(void) const
{
    mLastHallPosition = mHallDecoder.getCurrentHallState();
    prepareCommutation(mLastHallPosition);
    TIM_GenerateEvent(mHBridge.mTim.getBasePointer(), TIM_EventSource_COM);
}

void SensorBLDC::reverseTrigger(void) const
{
    mLastHallPosition = getPreviousHallPosition(mHallDecoder.getCurrentHallState());
    prepareCommutation(mLastHallPosition);
    TIM_GenerateEvent(mHBridge.mTim.getBasePointer(), TIM_EventSource_COM);
}

void SensorBLDC::checkMotor(const dev::Battery& battery) const
{
    const float blockingCurrent = 4; // [A]
    const float minimalRPS = 0.5;
    const uint32_t minimalPWMinMill = 80;

    const bool motorBlocking = (std::abs(battery.getCurrent()) > blockingCurrent) &&
                               (mHallDecoder.getCurrentRPS() < minimalRPS);

    if (motorBlocking) {
        reverseTrigger();
    }

    const bool motorNotStarting = (mHallDecoder.getCurrentRPS() < minimalRPS) &&
                                  (minimalPWMinMill < std::abs(mHBridge.getPulsWidthPerMill()));

    if (motorNotStarting) {
        trigger();
    }
}

void SensorBLDC::start(void) const
{
    mHallDecoder.registerCommutationCallback([this] {
                                                 prepareCommutation(mHallDecoder.getCurrentHallState());
                                             });

    mHallDecoder.registerHallEventCheckCallback([this] {
                                                    return checkHallEvent();
                                                });

    /* Internal connection from HallDecoder Timer to Motor Timer */
    TIM_SelectInputTrigger(mHBridge.mTim.getBasePointer(), TIM_TS_ITR2);

    /* Enable connection between HallDecoder Timer and Motor Timer */
    TIM_SelectCOM(mHBridge.mTim.getBasePointer(), ENABLE);
    mHBridge.mTim.enable();
    mHallDecoder.mTim.enable();

    setPulsWidthInMill(1);

    TIM_CtrlPWMOutputs(mHBridge.mTim.getBasePointer(), ENABLE);

    trigger();
    os::ThisTask::sleep(std::chrono::milliseconds(250));
}

void SensorBLDC::stop(void) const
{
    TIM_CtrlPWMOutputs(mHBridge.mTim.getBasePointer(), DISABLE);
    mHallDecoder.mTim.disable();
    mHBridge.mTim.disable();
    TIM_SelectCOM(mHBridge.mTim.getBasePointer(), DISABLE);

    mHallDecoder.unregisterHallEventCheckCallback();
    mHallDecoder.unregisterCommutationCallback();
}

uint32_t SensorBLDC::getNumberOfPolePairs(void) const
{
    return mHallDecoder.POLE_PAIRS;
}

constexpr std::array<const dev::SensorBLDC,
                     dev::SensorBLDC::Description::__ENUM__SIZE> dev::Factory<dev::SensorBLDC>::Container;
