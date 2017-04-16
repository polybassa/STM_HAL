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

#include "BalanceController.h"
#include "trace.h"
#include <cmath>
#include <Eigen/Dense>
#include "Gpio.h"

using app::BalanceController;
using app::DirectMotorController;
using app::Mpu;
using dev::PIDController;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

BalanceController::BalanceController(
                                     const Mpu&             gyro,
                                     DirectMotorController& motor) :
    os::DeepSleepModule(),
    mBalanceControllerTask("3BalanceControl",
                           BalanceController::STACKSIZE,
                           os::Task::Priority::HIGH,
                           [this](const bool& join)
                           {
                               balanceControllerTaskFunction(join);
                           }),
    mMpu(gyro),
    mMotor(motor),
    mSetAngleQueue(),
    mControllerP(mCurrentAngle,
                 mOutputTorqueControllerP,
                 mSetAngle,
                 Kp_P,
                 Ki_P,
                 Kd_P,
                 PIDController::ControlDirection::DIRECT),
    mControllerD(mCurrentAngleSpeed,
                 mOutputTorqueControllerD,
                 mSetAngleSpeed,
                 Kp_D,
                 Ki_D,
                 Kd_D,
                 PIDController::ControlDirection::DIRECT)
{
    mControllerD.setSampleTime(mControllerInterval);
    mControllerD.setOutputLimits(-1.0, 1.0); // Enter maximum torque here
    mControllerD.setMode(PIDController::ControlMode::AUTOMATIC);

    mControllerP.setSampleTime(mControllerInterval);
    mControllerP.setOutputLimits(-1.0, 1.0); // Enter maximum torque here
    mControllerP.setMode(PIDController::ControlMode::AUTOMATIC);
}

void BalanceController::enterDeepSleep(void)
{
    mBalanceControllerTask.join();
    mControllerP.setMode(PIDController::ControlMode::MANUAL);
    mControllerD.setMode(PIDController::ControlMode::MANUAL);
}

void BalanceController::exitDeepSleep(void)
{
    mControllerP.setMode(PIDController::ControlMode::AUTOMATIC);
    mControllerD.setMode(PIDController::ControlMode::AUTOMATIC);
    mBalanceControllerTask.start();
}

void BalanceController::balanceControllerTaskFunction(const bool& join)
{
    os::ThisTask::sleep(std::chrono::seconds(5));
    Trace(ZONE_INFO, "Start balance controller\r\n");
    do {
//        mSetAngleQueue.receive(mSetAngle, 0);

        auto eulerAngle = mMpu.getEuler();
        auto gyro = mMpu.getGyro();

        mCurrentAngle = (-eulerAngle(1));

        if (mCurrentAngle < 0) {mCurrentAngle += 180.0; } else {mCurrentAngle -= 180.0; }

        mCurrentAngleSpeed = (gyro);

        mControllerD.compute();
        mControllerP.compute();
        mMotor.setTorque(0.0 - (mOutputTorqueControllerP + mOutputTorqueControllerD));

//        terminal.print("x: %f, y: %f z: %f\r\n", mMpu.getGravity().x(),
//                 mMpu.getGravity().y(), mMpu.getGravity().z());
#ifdef DEBUG
        terminal.print("in: %f, CP_out: %f, CD_out: %f, angleSpeed: %f\r\n",
                       mCurrentAngle,
                       mOutputTorqueControllerP,
                       mOutputTorqueControllerD,
                       gyro);
#endif /*DEBUG */

        os::ThisTask::sleep(mControllerInterval);
    } while (!join);
}

void BalanceController::setTargetAngleInDegree(const float angle)
{
    static constexpr float limit = 20.0;

    if (std::abs(angle) < limit) {
        static constexpr float conversionFactor = 360.0 / (2 * M_PI);
        mSetAngleQueue.overwrite(angle / conversionFactor);
    }
}

#if defined(DEBUG)
void BalanceController::setTunings(const float kp, const float ki, const float kd)
{
    mControllerP.setTunings(kp, ki, kd);
}
#endif
