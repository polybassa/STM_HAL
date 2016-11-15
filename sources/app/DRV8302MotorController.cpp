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

DRV8302MotorController::DRV8302MotorController(const SensorBLDC& motor, const float Kp,
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
    mController(mCurrentTorque,
                mOutputTorque,
                mSetTorque,
                Kp,
                Ki,
                static_cast<const float>(0.0),
                dev::PIDController::ControlDirection::DIRECT),
    mSetTorqueQueue()
{
    mController.setSampleTime(controllerInterval);
    mController.setOutputLimits(-1.0, 1.0);
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
	// TODO MOVE CALIBRATION TO DIFFERENT PLACE
    mMotor.setPulsWidthInMill(0);
    os::ThisTask::sleep(controllerInterval);
    os::ThisTask::sleep(controllerInterval);
    auto offset1 = mMotor.mPhaseCurrentSensor.mAdcWithDma.getVoltage(mMotor.mPhaseCurrentSensor.mPhaseCurrentValue);
    os::ThisTask::sleep(controllerInterval);
    os::ThisTask::sleep(controllerInterval);
    auto offset2 = mMotor.mPhaseCurrentSensor.mAdcWithDma.getVoltage(mMotor.mPhaseCurrentSensor.mPhaseCurrentValue);
    os::ThisTask::sleep(controllerInterval);
    os::ThisTask::sleep(controllerInterval);
    auto offset3 = mMotor.mPhaseCurrentSensor.mAdcWithDma.getVoltage(mMotor.mPhaseCurrentSensor.mPhaseCurrentValue);
    mMotor.mPhaseCurrentSensor.setOffset((offset1 + offset2 + offset3) / 3.0);
    do {
        float newSetTorque;
        if (mSetTorqueQueue.receive(newSetTorque, 0)) {
            mSetTorque = newSetTorque;
        }
        auto phaseCurrent = mMotor.getPhaseCurrent();

        mCurrentTorque = phaseCurrent * mMotor.mMotorConstant;
        mController.compute();

        updatePwmOutput();
        updateQuadrant();

        g_RTTerminal->printf("Soll: %d\tIst: %d\tOutput: %d\tPWM: %d\tRPM: %d\tDIR: %s\tMode: %s\r\n",
                             static_cast<int32_t>(mSetTorque * 1000),
                             static_cast<int32_t>(mCurrentTorque * 1000),
                             static_cast<int32_t>(mOutputTorque * 1000),
                             mMotor.getPulsWidthPerMill(),
                             static_cast<int32_t>(mMotor.getCurrentRPS() * 60.0),
                             mMotor.getCurrentDirection() == SensorBLDC::Direction::FORWARD ? "FORWARD" : "BACKWARD",
                             mMotor.mManualCommutationActive ? "Manual" : "Automatic");

        mMotor.checkMotor();

        mPhaseCurrentValueAvailable.take(controllerInterval);
        os::ThisTask::sleep(controllerInterval);
        os::ThisTask::sleep(controllerInterval);
        os::ThisTask::sleep(controllerInterval);
    } while (!join);
}

void DRV8302MotorController::updateQuadrant(void)
{
    if (mSetTorque > 0) {
        mMotor.setDirection(dev::SensorBLDC::Direction::FORWARD);
    } else {
        mMotor.setDirection(dev::SensorBLDC::Direction::BACKWARD);
    }
}

void DRV8302MotorController::updatePwmOutput(void)
{
//    const float drivingCurrentInMotor = mOutputTorque / mMotor.mMotorConstant;
//    const float deltaVoltage = drivingCurrentInMotor * mMotor.mMotorCoilResistance;
//    const float voltageInductionMotor = mMotor.getCurrentOmega() * mMotor.mMotorConstant;
//    const float voltageInputMotor = voltageInductionMotor + deltaVoltage;
    //mSetPwm = (voltageInputMotor / 15) * 1000.0; // TODO remove 15. Stands for 15V on Motor
    mSetPwm = std::abs((mSetTorque * 1000));
    mMotor.setPulsWidthInMill(static_cast<int32_t>(mSetPwm));
    //mMotor.setPulsWidthInMill(static_cast<int32_t>(std::abs(mOutputTorque * 2000)));
}

void DRV8302MotorController::setTorque(const float setValue)
{
    mSetTorqueQueue.overwrite(setValue);
}

float DRV8302MotorController::getCurrentRPS(void) const
{
    return mMotor.getCurrentRPS();
}
