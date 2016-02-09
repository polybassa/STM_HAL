/* Copyright (C) 2015  Nils Weiss, Alexander Strobl, Daniel Tatzel
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
#include "PIDController.h"
#include "TimSensorBldc.h"
#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "Battery.h"
#include "os_Queue.h"
#include <limits>

#ifdef UNITTEST
extern int ut_TestPIDInput(void);
extern int ut_TestPIDOutput(void);
extern int ut_TestPIDInOut(void);
extern int ut_TestSetTorque(void);
#endif

namespace app
{
class MotorController final : private os::DeepSleepModule, public interface::MotorController<MotorController> {
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr uint32_t STACKSIZE = 1024;

    os::TaskInterruptable mMotorControllerTask;
    const dev::SensorBLDC& mMotor;
    const dev::Battery& mBattery;
    const float mMotorConstant = 0.00768;
    const float mMotorCoilResistance = 0.844;
    dev::PIDController mController;
    float mSetTorque = std::numeric_limits<float>::epsilon();
    float mCurrentTorque = std::numeric_limits<float>::epsilon();
    float mOutputTorque = std::numeric_limits<float>::epsilon();
    float mCurrentOmega = std::numeric_limits<float>::epsilon();

    const os::Queue<float, 1> mSetTorqueQueue;

    static constexpr std::chrono::milliseconds motorCheckInterval = std::chrono::milliseconds(1);
    static constexpr std::chrono::milliseconds controllerInterval = std::chrono::milliseconds(5);

    void motorControllerTaskFunction(const bool&);
    void updateCurrentTorque(void);
    void updatePwmOutput(void);

public:
    MotorController(
        const dev::SensorBLDC& motor,
        const dev::Battery&    battery,
        const float            motorConstant,
        const float            motorResistance,
        const float            Kp,
        const float            Ki);

    MotorController(const MotorController&) = delete;
    MotorController(MotorController&&) = delete;
    MotorController& operator=(const MotorController&) = delete;
    MotorController& operator=(MotorController&&) = delete;

    void setTorque(const float) const;
    float getCurrentRPS(void) const;

#if defined(DEBUG)
    void setTunings(const float kp, const float ki, const float kd);
#endif

#ifdef UNITTEST
    void triggerTaskExecution(void) { this->motorControllerTaskFunction(true); }
    friend int ::ut_TestPIDInput(void);
    friend int ::ut_TestPIDOutput(void);
    friend int ::ut_TestPIDInOut(void);
    friend int ::ut_TestSetTorque(void);
#endif
};
}

#endif /* SOURCES_PMD_MOTORCONTROLLER_H_ */
