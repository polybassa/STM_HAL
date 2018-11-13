// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include <chrono>

namespace dev
{
struct PIDController final {
    enum class ControlDirection {
        DIRECT,
        REVERSE
    };

    enum class ControlMode {
        AUTOMATIC,
        MANUAL
    };

    PIDController(float&                 input,
                  float&                 output,
                  float&                 setPoint,
                  const float            kp,
                  const float            ki,
                  const float            kd,
                  const ControlDirection direction);

    void setMode(const ControlMode);
    bool compute(void);
    void setOutputLimits(const float, const float);

    void setTunings(const float kp, const float ki, const float kd);
    void setControllerDirection(const ControlDirection);
    void setSampleTime(const std::chrono::milliseconds);

    float getKp(void) const;
    float getKi(void) const;
    float getKd(void) const;

    ControlDirection getDirection(void) const;
    ControlMode getMode(void) const;

private:
    PIDController(const PIDController&) = delete;
    PIDController(PIDController&&) = default;
    PIDController& operator=(const PIDController&) = delete;
    PIDController& operator=(PIDController&&) = delete;

    void initalize(void);
    void checkLimits(float&);

    float mDispKp;
    float mDispKi;
    float mDispKd;

    float mKp;
    float mKi;
    float mKd;

    ControlDirection mDirection;

    float& mInput;
    float& mOutput;
    float& mSetPoint;

    uint32_t mLastTime;
    float mITerm;
    float mLastInput;

    std::chrono::milliseconds mSampleTime = std::chrono::milliseconds(5);
    float mOutMin;
    float mOutMax;
    ControlMode mMode = ControlMode::MANUAL;
};
}
