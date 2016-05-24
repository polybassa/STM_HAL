/* Copyright (C) 2016  Nils Weiss
 # *
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

#ifndef SOURCES_PMD_DIRECTMOTORCONTROLLER_H_
#define SOURCES_PMD_DIRECTMOTORCONTROLLER_H_

#include "interface_MotorController.h"
#include "TimSensorBldc.h"
#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "Battery.h"
#include "os_Queue.h"
#include <limits>

namespace app
{
class DirectMotorController final :
    private os::DeepSleepModule, public interface::MotorController
{
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr uint32_t STACKSIZE = 1024;

    os::TaskInterruptable mMotorControllerTask;
    const dev::SensorBLDC& mMotor;
    const dev::Battery& mBattery;
    const float mMotorConstant = 0.0749; // [Nm/A]
    const float mMotorCoilResistance = 0.795; // [Ohm]
    const float mMotorCoilInductance = 0.000261; // [H]
    float mSetTorque = std::numeric_limits<float>::epsilon();
    float mSetPwm = std::numeric_limits<float>::epsilon();

    os::Queue<float, 1> mSetTorqueQueue;
    os::Queue<float, 1> mSetPwmQueue;

    static constexpr std::chrono::milliseconds motorCheckInterval = std::chrono::milliseconds(1);
    static constexpr std::chrono::milliseconds controllerInterval = std::chrono::milliseconds(2);

    void motorControllerTaskFunction(const bool&);
    void updatePwmOutput(void);
    void updateQuadrant(void);

public:
    DirectMotorController(
                          const dev::SensorBLDC & motor,
                          const dev::Battery & battery,
                          const float motorConstant,
                          const float motorResistance,
                          const float motorInductance);

    DirectMotorController(const DirectMotorController &) = delete;
    DirectMotorController(DirectMotorController &&) = delete;
    DirectMotorController& operator=(const DirectMotorController&) = delete;
    DirectMotorController& operator=(DirectMotorController &&) = delete;

    virtual void setTorque(const float) override;
    void setPwm(const float);
    virtual float getCurrentRPS(void) const override;

#ifdef UNITTEST
    void triggerTaskExecution(void) { this->motorControllerTaskFunction(true); }
#endif
};
}

#endif /* SOURCES_PMD_DIRECTMOTORCONTROLLER_H_ */
