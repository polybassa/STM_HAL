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
    mMotorControllerTask("DirectMotorControl",
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
            mMotor.checkMotor();
            os::ThisTask::sleep(motorCheckInterval);
        }
    } while (!join);
}

void DirectMotorController::updateQuadrant(void)
{
    if (mSetTorque > 0) {
        mMotor.setDirection(dev::SensorBLDC::Direction::FORWARD);
    } else {
        mMotor.setDirection(dev::SensorBLDC::Direction::BACKWARD);
    }
}

void DirectMotorController::updatePwm(void)
{
    mSetPwm = static_cast<int32_t>(mSetTorque * 1000);

    if (std::abs(mSetTorque) < 0.1) {
        mSetPwm = 0;
    }
}

void DirectMotorController::updatePwmOutput(void)
{
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
