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

#ifndef SOURCES_PMD_MOTORCONTROLLER_H_
#define SOURCES_PMD_MOTORCONTROLLER_H_

#include "interface_MotorController.h"
#include "TimSensorBldc.h"
#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "os_Queue.h"
#include "PIDController.h"
#include <limits>
#include "Battery.h"

namespace app
{
class MotorController final :
    private os::DeepSleepModule, public interface::MotorController
{
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr uint32_t STACKSIZE = 2048;

    os::TaskInterruptable mMotorControllerTask;
    const dev::SensorBLDC& mMotor;
    const dev::Battery& mBattery;
    float mSetTorque = std::numeric_limits<float>::epsilon();
    float mCurrentTorque = std::numeric_limits<float>::epsilon();
    float mOutputTorque = std::numeric_limits<float>::epsilon();
    float mSetPwm = std::numeric_limits<float>::epsilon();

    dev::PIDController mController;

    os::Queue<float, 1> mSetTorqueQueue;

    static constexpr std::chrono::milliseconds controllerInterval = std::chrono::milliseconds(4);

    void motorControllerTaskFunction(const bool&);
    void updatePwmOutput(void);
    void updateQuadrant(void);
    void adjustControllerLimits(void);

public:
    MotorController(const dev::SensorBLDC & motor, const dev::Battery & battery, const float Kp,
                    const float Ki);

    MotorController(const MotorController &) = delete;
    MotorController(MotorController &&) = delete;
    MotorController& operator=(const MotorController&) = delete;
    MotorController& operator=(MotorController &&) = delete;

    virtual void setTorque(const float) override;
    virtual float getCurrentRPS(void) const override;
    os::Semaphore mPhaseCurrentValueAvailable;

#ifdef UNITTEST
    void triggerTaskExecution(void) { this->motorControllerTaskFunction(true); }
#endif
};
}

#endif /* SOURCES_PMD_MOTORCONTROLLER_H_ */
