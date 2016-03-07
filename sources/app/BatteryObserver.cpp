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

#include "BatteryObserver.h"
#include <cstdio>
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using app::BatteryObserver;

const std::chrono::milliseconds BatteryObserver::energyRecordInterval = std::chrono::milliseconds(100);

BatteryObserver::BatteryObserver(const dev::Battery&                  battery,
                                 const std::function<void(ErrorCode)> errorCallback) :
    os::DeepSleepModule(),
    mEnergyRecordTask(
                      "2BatteryObserver",
                      BatteryObserver::STACKSIZE,
                      os::Task::Priority::LOW,
                      [this](
                             const
                             bool&
                             join) {
                          energyRecordTaskFunction(
                                                   join);
                      }),
    mErrorCallback(
                   errorCallback),
    mBattery(
             battery) {}

void BatteryObserver::enterDeepSleep(void)
{
    mEnergyRecordTask.join();
    mEnteredDeepSleep = hal::Rtc::now();
}

void BatteryObserver::exitDeepSleep(void)
{
    auto wakeupTimepoint = hal::Rtc::now();
    auto deepSleepTime = std::chrono::duration_cast<std::chrono::milliseconds>(wakeupTimepoint - mEnteredDeepSleep);

    auto deepSleepEnergy = calculateEnergyConsumption(deepSleepTime, powerConsumptionDuringDeepSleep);
    decreaseEnergyLevel(deepSleepEnergy);

    mEnergyRecordTask.start();
}

float BatteryObserver::calculateEnergyConsumption(const std::chrono::milliseconds duration, const float power) const
{
    return (power * duration.count()) /
           (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::hours(1)).count());
}

void BatteryObserver::overcurrentDetection(void)
{
    const bool overcurrent = mBattery.getCurrent() > BatteryObserver::limitOvercurrent;

    if (overcurrent) {
        mErrorCallback(ErrorCode::OVERCURRENT);
    }
}

void BatteryObserver::undervoltageDetection(void)
{
    const bool undervoltage = mBattery.getVoltage() < BatteryObserver::limitUndervoltage;

    if (undervoltage) {
        mErrorCallback(ErrorCode::UNDERVOLTAGE);
    }
}

void BatteryObserver::energyRecordTaskFunction(const bool& join)
{
    /* use do while here to manually trigger execution in unit test */
    do {
        overcurrentDetection();
        undervoltageDetection();

        auto ticksNow = os::Task::getTickCount();
        auto intervalDuration = std::chrono::milliseconds(ticksNow - mLastRecordTimestamp);
        mLastRecordTimestamp = ticksNow;

        auto measuredEnergy = calculateEnergyConsumption(intervalDuration, mBattery.getPower());

        if (measuredEnergy < 0) {
            decreaseEnergyLevel(measuredEnergy);
        } else {
            increaseEnergyLevel(measuredEnergy);
        }

        os::ThisTask::sleep(BatteryObserver::energyRecordInterval);
    } while (!join);
}

void BatteryObserver::decreaseEnergyLevel(const float energy)
{
    mEnergy += energy;
    if (mEnergy <= 0.0) {
        mEnergy = 0;
    }
}

void BatteryObserver::increaseEnergyLevel(const float energy)
{
    mEnergy += energy;
    if (mEnergy > mMaxEnergy) {
        mMaxEnergy = mEnergy;
    }
}

float BatteryObserver::getEnergyLevelInPercent(void) const
{
    return (mEnergy * 100) / mMaxEnergy;
}

float BatteryObserver::getEnergy(void) const
{
    return mEnergy;
}

float BatteryObserver::getMaxEnergy(void) const
{
    return mMaxEnergy;
}
