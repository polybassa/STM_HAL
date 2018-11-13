// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include <functional>
#include "Battery.h"
#include "TaskInterruptable.h"
#include "Rtc.h"
#include "DeepSleepInterface.h"

namespace app
{
struct BatteryObserver final :
    private os::DeepSleepModule {
    enum class ErrorCode {
        OVERCURRENT = 0,
        UNDERVOLTAGE,
        OVERVOLTAGE,
        BATTERY_EMPTY,
        BATTERY_ALMOST_EMPTY
    };

    BatteryObserver(const dev::Battery&, const std::function<void(ErrorCode)> errorCallback);

    BatteryObserver(const BatteryObserver&) = delete;
    BatteryObserver(BatteryObserver&&) = delete;
    BatteryObserver& operator=(const BatteryObserver&) = delete;
    BatteryObserver& operator=(BatteryObserver&&) = delete;

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
                                     const float) const;
    void overcurrentDetection(void);
    void undervoltageDetection(void);
    void energyRecordTaskFunction(const bool&);
    void decreaseEnergyLevel(const float);
    void increaseEnergyLevel(const float);
};
}
