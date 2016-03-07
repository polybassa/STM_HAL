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

#ifndef SOURCES_PMD_BATTERYOBSERVER_H_
#define SOURCES_PMD_BATTERYOBSERVER_H_

#include <functional>
#include "Battery.h"
#include "TaskInterruptable.h"
#include "Rtc.h"
#include "DeepSleepInterface.h"

namespace app
{
struct BatteryObserver final :
    private os::DeepSleepModule {
    enum class ErrorCode
    {
        OVERCURRENT = 0,
        UNDERVOLTAGE,
        OVERVOLTAGE,
        BATTERY_EMPTY,
        BATTERY_ALMOST_EMPTY
    };

    BatteryObserver(const dev::Battery &, const std::function<void(ErrorCode)> errorCallback);

    BatteryObserver(const BatteryObserver &) = delete;
    BatteryObserver(BatteryObserver &&) = delete;
    BatteryObserver& operator=(const BatteryObserver&) = delete;
    BatteryObserver& operator=(BatteryObserver &&) = delete;

    float getEnergyLevelInPercent(void) const;
    float getEnergy(void) const;
    float getMaxEnergy(void) const;

#ifdef UNITTEST
    void triggerTaskExecution(void) { this->energyRecordTaskFunction(true); }
#endif
private:

    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    os::TaskInterruptable mEnergyRecordTask;
    const std::function<void(ErrorCode)> mErrorCallback;
    const dev::Battery& mBattery;
    float mEnergy = 0;
    float mMaxEnergy = 0;
    hal::Rtc::time_point mEnteredDeepSleep;
    uint32_t mLastRecordTimestamp = 0;

    static constexpr const uint32_t STACKSIZE = 1024;
    static constexpr const float limitOvercurrent = 40.0;
    static constexpr const float limitUndervoltage = 5;
    static constexpr const float powerConsumptionDuringDeepSleep = -0.1;
    static const std::chrono::milliseconds energyRecordInterval;

    float calculateEnergyConsumption(const std::chrono::milliseconds,
                                     const              float) const;
    void overcurrentDetection(void);
    void undervoltageDetection(void);
    void energyRecordTaskFunction(const bool&);
    void decreaseEnergyLevel(const float);
    void increaseEnergyLevel(const float);
};
}

#endif /* SOURCES_PMD_BATTERYOBSERVER_H_ */
