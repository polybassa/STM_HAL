/* Copyright (C) 2015  Nils Weiss, Alexander Strobl, Daniel Tatzel
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

#include "MotorController.h"
#include "trace.h"
#include <cmath>

using app::MotorController;
using dev::Battery;
using dev::PIDController;
using dev::SensorBLDC;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

constexpr std::chrono::milliseconds MotorController::motorCheckInterval;
constexpr std::chrono::milliseconds MotorController::controllerInterval;

MotorController::MotorController(
    const SensorBLDC& motor,
    const Battery&    battery,
    const float       motorConstant,
    const float       motorResistance,
    const float       Kp,
    const float       Ki) : os::DeepSleepModule(),
    mMotorControllerTask("1MotorControl",
                         MotorController::STACKSIZE,
                         os::Task::Priority::HIGH,
                         [this](const bool& join)
{
    motorControllerTaskFunction(join);
}),
    mMotor(motor),
    mBattery(battery),
    mMotorConstant(motorConstant),
    mMotorCoilResistance(motorResistance),
    mController(mCurrentTorque,
                mOutputTorque,
                mSetTorque,
                Kp,
                Ki,
                static_cast<const float>(0.0),
                PIDController::ControlDirection::DIRECT),
    mSetTorqueQueue()
{
    mController.setSampleTime(controllerInterval);
    mController.setOutputLimits(-1.0, 1.0);
    mController.setMode(PIDController::ControlMode::AUTOMATIC);
    mSetTorque = 0.00001;
    mMotor.start();
}

void MotorController::enterDeepSleep(void)
{
    mMotorControllerTask.join();
    mController.setMode(PIDController::ControlMode::MANUAL);
    mMotor.stop();
}

void MotorController::exitDeepSleep(void)
{
    mMotor.start();
    mController.setMode(PIDController::ControlMode::AUTOMATIC);
    mMotorControllerTask.start();
}

void MotorController::motorControllerTaskFunction(const bool& join)
{
    do {
        float newSetTorque;
        if (mSetTorqueQueue.receive(newSetTorque, 0)) {
            mSetTorque = newSetTorque;
        }
        mCurrentOmega = mMotor.getCurrentOmega();
        updateCurrentTorque();
        mController.compute();
        updatePwmOutput();
        static constexpr uint32_t waitPeriode = controllerInterval.count() / motorCheckInterval.count();
        for (uint32_t i = 0; i < waitPeriode; i++) {
            mMotor.checkMotor();
            os::ThisTask::sleep(motorCheckInterval);
        }
    } while (!join);
}

#if defined(DEBUG)
void MotorController::setTunings(const float kp, const float ki, const float kd)
{
    mController.setTunings(kp, ki, kd);
}
#endif

void MotorController::updateCurrentTorque(void)
{
    /*
     * P = T * Omega
     * P = U * I
     * I = Omega * MotorConstant
     */
    const float voltageInputMotor = (static_cast<float>(mMotor.getPulsWidthPerMill()) / 1000.0) * mBattery.getVoltage();
    const float voltageInductionMotor = mCurrentOmega * mMotorConstant;
    const float deltaVoltage = voltageInputMotor - voltageInductionMotor; // bessere Formel mit Induktivitaet und Frequenz verwenden
    const float drivingCurrentInMotor = deltaVoltage / mMotorCoilResistance;
    mCurrentTorque = drivingCurrentInMotor * mMotorConstant;
}

void MotorController::updatePwmOutput(void)
{
    const float drivingCurrentInMotor = mOutputTorque / mMotorConstant;
    const float deltaVoltage = drivingCurrentInMotor * mMotorCoilResistance;
    const float voltageInductionMotor = mCurrentOmega * mMotorConstant;
    const float voltageInputMotor = voltageInductionMotor + deltaVoltage;
    const float pulswidth = (voltageInputMotor / mBattery.getVoltage()) * 1000.0;
    mMotor.setPulsWidthInMill(pulswidth);
}

void MotorController::setTorque(const float setValue)
{
    mSetTorqueQueue.overwrite(setValue);
}

float MotorController::getCurrentRPS(void) const
{
    return mMotor.getCurrentRPS();
}
