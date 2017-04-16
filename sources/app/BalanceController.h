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

#ifndef SOURCES_PMD_BALANCECONTROLLER_H_
#define SOURCES_PMD_BALANCECONTROLLER_H_

#include "PIDController.h"
#include "DirectMotorController.h"
#include "Mpu.h"
#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "interface_BalanceController.h"
#include "os_Queue.h"
#include <limits>

namespace app
{
class BalanceController final :
    private os::DeepSleepModule, public interface::BalanceController
{
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr uint32_t STACKSIZE = 2048;

    os::TaskInterruptable mBalanceControllerTask;
    const Mpu& mMpu;
    DirectMotorController& mMotor;
    os::Queue<float, 1> mSetAngleQueue;
    dev::PIDController mControllerP;
    dev::PIDController mControllerD;

    float mSetAngle = 0;
    float mCurrentAngle = std::numeric_limits<float>::epsilon();
    float mOutputTorqueControllerD = std::numeric_limits<float>::epsilon();

    float mSetAngleSpeed = 0;
    float mCurrentAngleSpeed = std::numeric_limits<float>::epsilon();
    float mOutputTorqueControllerP = std::numeric_limits<float>::epsilon();

    static constexpr float Kp_P = 0.009;
    static constexpr float Ki_P = 0;
    static constexpr float Kd_P = 0;

    static constexpr float Kp_D = 0.009;
    static constexpr float Ki_D = 0;
    static constexpr float Kd_D = 0;

    const std::chrono::milliseconds mControllerInterval = std::chrono::milliseconds(6);

    void balanceControllerTaskFunction(const bool&);
public:
    BalanceController(
                      const Mpu &gyro,
                      DirectMotorController & motor);

    BalanceController(const BalanceController &) = delete;
    BalanceController(BalanceController &&) = delete;
    BalanceController& operator=(const BalanceController&) = delete;
    BalanceController& operator=(BalanceController &&) = delete;

    virtual void setTargetAngleInDegree(const float angle) override;

#if defined(DEBUG)
    void setTunings(const float kp, const float ki, const float kd);
#endif
};
}

#endif /* SOURCES_PMD_BALANCECONTROLLER_H_ */
