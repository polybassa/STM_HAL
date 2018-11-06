/* Copyright (C) 2015  Nils Weiss, Alexander Strobl
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
