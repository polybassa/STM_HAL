// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "os_Queue.h"
#include "virtual_BalanceController.h"
#include "BalanceController.h"
#include "StraingaugeSensor.h"
#include <limits>

namespace app
{
class SteeringController final :
    private os::DeepSleepModule
{
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr uint32_t STACKSIZE = 1024;

    os::TaskInterruptable mSteeringControllerTask;
    os::TaskInterruptable mStraingaugeUpdateTask;

    os::Queue<std::pair<float, float>, 1> mQueue;
    os::Queue<float, 1> mStraingaugeValueQueue;

    BalanceController& mMasterBalance;
    virt::BalanceController& mSlaveBalance;
    const dev::StraingaugeSensor& mDms;

    void SteeringControllerTaskFunction(const bool&);
    void StraingaugeUpdateTaskFunction(const bool&);

public:
    SteeringController(
                       BalanceController&            masterBalance,
                       virt::BalanceController&      slaveBalance,
                       const dev::StraingaugeSensor& dms);

    SteeringController(const SteeringController&) = delete;
    SteeringController(SteeringController&&) = delete;
    SteeringController& operator=(const SteeringController&) = delete;
    SteeringController& operator=(SteeringController&&) = delete;

    void setDirectionAndAngleInDegree(const float, const float);
    void setDirectionAndAngleInDegree(const std::pair<float, float> );
    void setAngleInDegree(const float);
};
}
