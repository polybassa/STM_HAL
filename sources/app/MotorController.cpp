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

#include "MotorController.h"
#include "trace.h"
#include <cmath>

using app::MotorController;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

constexpr std::chrono::milliseconds MotorController::controllerInterval;

MotorController::MotorController(const dev::SensorBLDC& motor,
                                 const dev::Battery&    battery,
                                 const float            Kp,
                                 const float            Ki) :
    os::DeepSleepModule(),
    mMotorControllerTask("MotorControl",
                         MotorController::STACKSIZE,
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
                dev::PIDController::ControlDirection::DIRECT),
    mSetTorqueQueue()
{
    mController.setSampleTime(controllerInterval);
    mController.setOutputLimits(-100000, 100000);
    mController.setMode(dev::PIDController::ControlMode::AUTOMATIC);
    setTorque(0.00001);
    mSetTorque = 0.00001;

    mMotor.mPhaseCurrentSensor.registerValueAvailableSemaphore(&mPhaseCurrentValueAvailable);

    mMotor.start();
}

void MotorController::enterDeepSleep(void)
{
    mMotorControllerTask.join();
    mMotor.stop();
}

void MotorController::exitDeepSleep(void)
{
    mMotor.start();
    mMotorControllerTask.start();
}

void MotorController::motorControllerTaskFunction(const bool& join)
{
    mMotor.calibrate();
    mMotor.setPulsWidthInMill(0);

    do {
        float newSetTorque;
        if (mSetTorqueQueue.receive(newSetTorque, 0)) {
            mSetTorque = newSetTorque;
        }

        mCurrentTorque = mMotor.getActualTorqueInNewtonMeter();
        adjustControllerLimits();
        mController.compute();
        updatePwmOutput();
        updateQuadrant();

#if 0
        TraceLight(
                   "tick: %5d\t"
                   "Soll: %5d\t"
                   "Out: %5d\t"
                   "Ist: %5d\t"
                   "PWM: %5d\t"
                   "RPS: %5d\t"
                   //"periode: %5d\t"
                   //"samples: %5d\t"
                   "U[mV]: %5d\t"
                   //"Phase I: %5d\t"
                   "CDIR: %s\t"
                   "SDIR: %s\t"
                   "\n",
                   os::Task::getTickCount() % 1000,
                   static_cast<int32_t>(mSetTorque * 1000),
                   static_cast<int32_t>(mOutputTorque * 1000),
                   static_cast<int32_t>(mCurrentTorque * 1000),
                   static_cast<int32_t>(mMotor.getActualPulsWidthPerMill()),
                   static_cast<int32_t>(mMotor.getActualRPS()),
                   //mMotor.mHBridge.mTim.getPeriode(),
                   //mMotor.mPhaseCurrentSensor.getNumberOfMeasurementsForPhaseCurrentValue()
                   static_cast<int32_t>(mBattery.getVoltage() * 1000),
                   //static_cast<int32_t>(mMotor.mPhaseCurrentSensor.getCurrentVoltage() * 1000),
                   mMotor.getActualDirection() == dev::SensorBLDC::Direction::FORWARD ? "F" : "B",
                   mMotor.getSetDirection() == dev::SensorBLDC::Direction::FORWARD ? "F" : "B"
                   );
#endif
        mMotor.checkMotor();
        mPhaseCurrentValueAvailable.take(controllerInterval);
    } while (!join);
}

void MotorController::adjustControllerLimits(void)
{
    const float MAXIMUM_VOLTAGE = mBattery.getVoltage();
    static constexpr const float MAXIMUM_PULSWIDTH = 1000.0; //because of pwm per mill
    const float MAXIMUM_CONTROLLER_OUTPUT = MAXIMUM_VOLTAGE * MAXIMUM_PULSWIDTH / mMotor.mMotorCoilResistance;

    /*
     * Formula for suppression factor = -0.03 / (abs(x) - 0.05)^2 + 1.1
     * suppression factor in range 0.01 to 1
     */

    float supressionFactor = -0.03 / std::pow(std::abs(mSetTorque) + 0.05, 2.0) + 1.1;

    //check bounds
    supressionFactor = std::min(1.0f, std::max(0.01f, supressionFactor));

    mController.setOutputLimits(0.0 - (MAXIMUM_CONTROLLER_OUTPUT * supressionFactor),
                                MAXIMUM_CONTROLLER_OUTPUT * supressionFactor);

    static const float initial_P = mController.getKp();
    static const float initial_I = mController.getKi();

    mController.setTunings(initial_P * supressionFactor, initial_I * supressionFactor, 0);
}

void MotorController::updateQuadrant(void)
{
    if (mOutputTorque < 0) {
        mMotor.setDirection(dev::SensorBLDC::Direction::FORWARD);
    } else {
        mMotor.setDirection(dev::SensorBLDC::Direction::BACKWARD);
    }
}

void MotorController::updatePwmOutput(void)
{
    /*
       dM / dPWM = U / R
       dPWM / dM = R / U
       dPWM = R * dM / U
     */

    mMotor.setPulsWidthInMill(static_cast<int32_t>(mMotor.mMotorCoilResistance * mOutputTorque /
                                                   mBattery.getVoltage()));
}

void MotorController::setTorque(const float setValue)
{
    mSetTorqueQueue.overwrite(setValue);
}

float MotorController::getCurrentRPS(void) const
{
    return mMotor.getActualRPS();
}
