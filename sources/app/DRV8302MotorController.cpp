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
#include "RealTimeDebugInterface.h"

using app::DRV8302MotorController;
using dev::SensorBLDC;

extern dev::RealTimeDebugInterface* g_RTTerminal;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

constexpr std::chrono::milliseconds DRV8302MotorController::controllerInterval;

DRV8302MotorController::DRV8302MotorController(const SensorBLDC& motor, const dev::Battery& battery, const float Kp,
                                               const float Ki) :
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
    mController.setOutputLimits(-1.0, 1.0);
    mController.setMode(dev::PIDController::ControlMode::AUTOMATIC);
    setTorque(0.00001);
    mSetTorque = 0.00001;

    mMotor.mPhaseCurrentSensor.registerValueAvailableSemaphore(&mPhaseCurrentValueAvailable, false);

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

    const uint8_t arrSize = 20;
    float arr[arrSize] = { 0 };
    static uint8_t counter = 0;

    do {
        float newSetTorque;
        if (mSetTorqueQueue.receive(newSetTorque, 0)) {
            mSetTorque = newSetTorque;
        }
        const auto phaseCurrent = mMotor.getPhaseCurrent();

        mCurrentTorque = phaseCurrent * mMotor.mMotorConstant * 3;

        arr[counter % arrSize] = mCurrentTorque * 1000;
        counter++;

        auto sum = 0;

        for (int i = 0; i < arrSize; i++) {
            sum += arr[i];
        }

        float avg = sum / arrSize;

//        mController.compute();
//
//        updatePwmOutput();
//        updateQuadrant();

        mMotor.setPulsWidthInMill(mSetTorque);

        g_RTTerminal->printf("%10d\t"
                             "Soll: %5d\t"
                             "Out: %5d\t"
                             "Ist: %5d\t"
                             "IstAvg: %5d\t"
                             "I: %5d\t"
                             "\n",
                             os::Task::getTickCount(),
                             static_cast<int32_t>(mSetTorque * 1000),
                             static_cast<int32_t>(mOutputTorque * 1000),
                             static_cast<int32_t>(mCurrentTorque * 1000),
                             static_cast<int32_t>(avg),
                             static_cast<int32_t>(phaseCurrent * 1000)
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
    const float voltageInductionMotor = std::abs(mMotor.getCurrentRPS() * 60) / mMotor.mMotorGeneratorConstant;
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
    return mMotor.getCurrentRPS();
}
