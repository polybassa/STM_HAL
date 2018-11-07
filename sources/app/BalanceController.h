// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "PIDController.h"
#include "MotorController.h"
#include "Mpu.h"
#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "interface_BalanceController.h"
#include "os_Queue.h"
#include <limits>

namespace app
{
class BalanceController final :
    private os::DeepSleepModule, public interface ::BalanceController
{
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr uint32_t STACKSIZE = 1024;

    os::TaskInterruptable mBalanceControllerTask;
    const Mpu& mMpu;
    MotorController& mMotor;
    os::Queue<float, 1> mSetAngleQueue;

    const std::chrono::milliseconds mControllerInterval = std::chrono::milliseconds(4);

    void balanceControllerTaskFunction(const bool&);
public:
    BalanceController(
                      const Mpu&       gyro,
                      MotorController& motor);

    BalanceController(const BalanceController&) = delete;
    BalanceController(BalanceController&&) = delete;
    BalanceController& operator=(const BalanceController&) = delete;
    BalanceController& operator=(BalanceController&&) = delete;

    virtual void setTargetAngleInDegree(const float angle) override;
};
}
