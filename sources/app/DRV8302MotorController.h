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

#ifndef SOURCES_PMD_DRV8302MOTORCONTROLLER_H_
#define SOURCES_PMD_DRV8302MOTORCONTROLLER_H_

#include "interface_MotorController.h"
#include "TimSensorBldc.h"
#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "os_Queue.h"
#include "PIDController.h"
#include <limits>

namespace app
{
class DRV8302MotorController final :
    private os::DeepSleepModule, public interface::MotorController
{
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr uint32_t STACKSIZE = 1024;

    os::TaskInterruptable mMotorControllerTask;
    const dev::SensorBLDC& mMotor;
    float mSetTorque = std::numeric_limits<float>::epsilon();
    float mCurrentTorque = std::numeric_limits<float>::epsilon();
    float mOutputTorque = std::numeric_limits<float>::epsilon();
    float mSetPwm = std::numeric_limits<float>::epsilon();

    dev::PIDController mController;

    os::Queue<float, 1> mSetTorqueQueue;
    os::Semaphore mPhaseCurrentValueAvailable;

    static constexpr std::chrono::milliseconds controllerInterval = std::chrono::milliseconds(10);
    // TODO set value to a smaller number of ms

    void motorControllerTaskFunction(const bool&);
    void updatePwmOutput(void);
    void updateQuadrant(void);

public:
    DRV8302MotorController(const dev::SensorBLDC & motor, const float Kp,
                           const float Ki);

    DRV8302MotorController(const DRV8302MotorController &) = delete;
    DRV8302MotorController(DRV8302MotorController &&) = delete;
    DRV8302MotorController& operator=(const DRV8302MotorController&) = delete;
    DRV8302MotorController& operator=(DRV8302MotorController &&) = delete;

    virtual void setTorque(const float) override;
    virtual float getCurrentRPS(void) const override;

#ifdef UNITTEST
    void triggerTaskExecution(void) { this->motorControllerTaskFunction(true); }
#endif
};
}

#endif /* SOURCES_PMD_DRV8302MOTORCONTROLLER_H_ */
