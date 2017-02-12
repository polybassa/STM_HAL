/* Copyright (C) 2016  Nils Weiss
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

#include "DRV8302MotorController.h"
#include "trace.h"
#include <cmath>

using app::DRV8302MotorController;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

constexpr std::chrono::milliseconds DRV8302MotorController::controllerInterval;

DRV8302MotorController::DRV8302MotorController(const dev::SensorBLDC& motor,
                                               const dev::Battery&    battery,
                                               const float            Kp,
                                               const float            Ki) :
    os::DeepSleepModule(),
    mMotorControllerTask("DRV8302MotorControl",
                         DRV8302MotorController::STACKSIZE,
                         os::Task::Priority::HIGH,
                         [this](const bool& join)
                         {
                             motorControllerTaskFunction(join);
                         }),
    mMotor(motor),
    mBattery(battery),
    mController(mCurrentTorque,
                mOutputTorque,
                mSetTorque,
                Kp,
                Ki,
                static_cast<const float>(0.0),
                dev::PIDController::ControlDirection::REVERSE),
    mSetTorqueQueue()
{
    mController.setSampleTime(controllerInterval);
    mController.setOutputLimits(-1.5, 1.5);
    mController.setMode(dev::PIDController::ControlMode::AUTOMATIC);
    setTorque(0.00001);
    mSetTorque = 0.00001;

    mMotor.mPhaseCurrentSensor.registerValueAvailableSemaphore(&mPhaseCurrentValueAvailable);

    mMotor.start();
}

void DRV8302MotorController::enterDeepSleep(void)
{
    mMotorControllerTask.join();
    mMotor.stop();
}

void DRV8302MotorController::exitDeepSleep(void)
{
    mMotor.start();
    mMotorControllerTask.start();
}

void DRV8302MotorController::motorControllerTaskFunction(const bool& join)
{
    mMotor.calibrate();
    mMotor.setPulsWidthInMill(0);

    do {
        float newSetTorque;
        if (mSetTorqueQueue.receive(newSetTorque, 0)) {
            mSetTorque = newSetTorque;
        }

        mCurrentTorque = mMotor.getActualTorqueInNewtonMeter();
        mController.compute();

        updatePwmOutput();
        updateQuadrant();

        Trace(ZONE_INFO, "%10d\t"
              "Soll: %5d\t"
              "Out: %5d\t"
              "Ist: %5d\t"
              "PWM: %5d\t"
              "RPS: %5d\t"
              "CDIR: %s\t"
              "SDIR: %s\t"
              "\n",
              os::Task::getTickCount(),
              static_cast<int32_t>(mSetTorque * 1000),
              static_cast<int32_t>(mOutputTorque * 1000),
              static_cast<int32_t>(mCurrentTorque * 1000),
              static_cast<int32_t>(mMotor.getActualPulsWidthPerMill()),
              static_cast<int32_t>(mMotor.getActualRPS()),
              mMotor.getActualDirection() == dev::SensorBLDC::Direction::FORWARD ? "F" : "B",
              mMotor.getSetDirection() == dev::SensorBLDC::Direction::FORWARD ? "F" : "B"
              );

        mMotor.checkMotor();
        mPhaseCurrentValueAvailable.take(controllerInterval);
    } while (!join);
}

void DRV8302MotorController::updateQuadrant(void)
{
    if (mOutputTorque > 0) {
        mMotor.setDirection(dev::SensorBLDC::Direction::FORWARD);
    } else {
        mMotor.setDirection(dev::SensorBLDC::Direction::BACKWARD);
    }
}

void DRV8302MotorController::updatePwmOutput(void)
{
    const float drivingCurrentInMotor = std::abs(mOutputTorque) / mMotor.mMotorConstant;
    const float deltaVoltage = drivingCurrentInMotor * mMotor.mMotorCoilResistance;
    const float voltageInductionMotor = std::abs(mMotor.getActualRPS() * 60) / mMotor.mMotorGeneratorConstant;
    const float voltageInputMotor = voltageInductionMotor + deltaVoltage;
    mSetPwm = (voltageInputMotor / mBattery.getVoltage()) * 1000.0;

    mMotor.setPulsWidthInMill(static_cast<int32_t>(mSetPwm));
}

void DRV8302MotorController::setTorque(const float setValue)
{
    mSetTorqueQueue.overwrite(setValue);
}

float DRV8302MotorController::getCurrentRPS(void) const
{
    return mMotor.getActualRPS();
}
