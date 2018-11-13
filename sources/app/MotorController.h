// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

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
    private os::DeepSleepModule, public interface ::MotorController
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
    MotorController(const dev::SensorBLDC& motor, const dev::Battery& battery, const float Kp,
                    const float Ki);

    MotorController(const MotorController&) = delete;
    MotorController(MotorController&&) = delete;
    MotorController& operator=(const MotorController&) = delete;
    MotorController& operator=(MotorController&&) = delete;

    virtual void setTorque(const float) override;
    virtual float getCurrentRPS(void) const override;
    os::Semaphore mPhaseCurrentValueAvailable;

#ifdef UNITTEST
    void triggerTaskExecution(void) { this->motorControllerTaskFunction(true); }
#endif
};
}
