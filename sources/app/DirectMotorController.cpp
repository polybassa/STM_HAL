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

#include "DirectMotorController.h"
#include "trace.h"
#include <cmath>

using app::DirectMotorController;
using dev::Battery;
using dev::SensorBLDC;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

constexpr std::chrono::milliseconds DirectMotorController::motorCheckInterval;
constexpr std::chrono::milliseconds DirectMotorController::controllerInterval;

DirectMotorController::DirectMotorController(
                                             const SensorBLDC& motor,
                                             const Battery&    battery,
                                             const float       motorConstant,
                                             const float       motorResistance,
                                             const float       motorInductance) :
    os::DeepSleepModule(),
    mMotorControllerTask("1DirectMotorControl",
                         DirectMotorController::STACKSIZE,
                         os::Task::Priority::HIGH,
                         [this](const bool& join)
                         {
                             motorControllerTaskFunction(join);
                         }),
    mMotor(motor),
    mBattery(battery),
    mMotorConstant(motorConstant),
    mMotorCoilResistance(motorResistance),
    mMotorCoilInductance(motorInductance),
    mSetTorqueQueue()
{
    setTorque(0.00001);
    mSetTorque = 0.00001;
    mMotor.start();
}

void DirectMotorController::enterDeepSleep(void)
{
    mMotorControllerTask.join();
    mMotor.stop();
}

void DirectMotorController::exitDeepSleep(void)
{
    mMotor.start();
    mMotorControllerTask.start();
}

void DirectMotorController::motorControllerTaskFunction(const bool& join)
{
    do {
        float newSetTorque;
        if (mSetTorqueQueue.receive(newSetTorque, 0)) {
            mSetTorque = newSetTorque;
        }
        updatePwm();
        updateQuadrant();
        updatePwmOutput();

        static constexpr uint32_t waitPeriode = controllerInterval.count() / motorCheckInterval.count();
        for (uint32_t i = 0; i < waitPeriode; i++) {
            mMotor.checkMotor(mBattery);
            os::ThisTask::sleep(motorCheckInterval);
        }
    } while (!join);
}

void DirectMotorController::updateQuadrant(void)
{
    const float omega = mMotor.getCurrentOmega();
    if (omega > 0) {
        if (mSetTorque > 0) {
            mMotor.setMode(dev::SensorBLDC::Mode::ACCELERATE);
        } else {
            if (mSetPwm > 0) {
                mMotor.setMode(dev::SensorBLDC::Mode::REGEN_BRAKE);
            } else {
                mMotor.setMode(dev::SensorBLDC::Mode::ACTIVE_BRAKE);
            }
        }
    } else {
        if (mSetTorque > 0) {
            if (mSetPwm > 0) {
                mMotor.setMode(dev::SensorBLDC::Mode::ACTIVE_BRAKE);
            } else {
                mMotor.setMode(dev::SensorBLDC::Mode::REGEN_BRAKE);
            }
        } else {
            mMotor.setMode(dev::SensorBLDC::Mode::ACCELERATE);
        }
    }
}

void DirectMotorController::updatePwm(void)
{
    const float omega = mMotor.getCurrentOmega();
    static const float cphi = mMotorConstant;
    static const float R = mMotorCoilResistance;
    float voltageInputMotor = 0.0;

    voltageInputMotor = (mSetTorque / cphi) * R + omega * cphi;

    mSetPwm = static_cast<int32_t>((voltageInputMotor / mBattery.getVoltage()) * 1000.0);

//    terminal.print("sqrt(");
//    terminal.print("%f * (%f * %f + %f", omega*omega, A, mSetTorque* mSetTorque, cphi2);
//    terminal.print(") + %f * %f +", B, mSetTorque * mSetTorque);
//    terminal.print("2 * %f * %f * %f)", mMotorCoilResistance, std::abs(mSetTorque), omega);
//    terminal.print(" = %f => %d pwm", voltageInputMotor, pulswidth / 10);
//    terminal.print("\r\n");
}

void DirectMotorController::updatePwmOutput(void)
{
    const float omega = mMotor.getCurrentOmega();

    if (mMotor.getMode() == dev::SensorBLDC::Mode::REGEN_BRAKE) {
        static const float cphi = mMotorConstant;
        static const float R = mMotorCoilResistance;
        float voltageInputMotor = 0.0;

        voltageInputMotor = ((mSetTorque * R)/(omega * cphi* cphi));
        mSetPwm = static_cast<int32_t>((voltageInputMotor) * 1000.0);
    }

    if (omega > -0.1 && omega < 0.1){

        	if (omega < 0 && mSetTorque > 0) {
        		mSetPwm *= -1;
        	} else if (omega > 0 && mSetTorque < 0) {
        		mSetPwm *= -1;
        	}
        }


    mMotor.setPulsWidthInMill(static_cast<int32_t>(std::abs(mSetPwm)));
}

void DirectMotorController::setTorque(const float setValue)
{
    mSetTorqueQueue.overwrite(setValue);
}

void DirectMotorController::setPwm(const float value)
{
    mSetPwmQueue.overwrite(value);
}

float DirectMotorController::getCurrentRPS(void) const
{
    return mMotor.getCurrentRPS();
}
