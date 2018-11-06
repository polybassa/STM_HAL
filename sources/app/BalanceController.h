/* Copyright (C) 2016 Nils Weiss
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
