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

#include "PIDController.h"
#include "os_Task.h"

using dev::PIDController;

PIDController::PIDController(float&                 input,
                             float&                 output,
                             float&                 setPoint,
                             const float            kp,
                             const float            ki,
                             const float            kd,
                             const ControlDirection direction) :
    mInput(input),
    mOutput(output),
    mSetPoint(setPoint),
    mLastTime(0)
{
    setOutputLimits(0, 255);
    setControllerDirection(direction);
    setTunings(kp, ki, kd);
}

bool PIDController::compute(void)
{
    if (mMode == ControlMode::MANUAL) {return false; }

    auto now = os::Task::getTickCount();
    auto timeChange = now - mLastTime;

    if (timeChange < mSampleTime.count()) {
        return false;
    }

    float input = mInput;
    float error = mSetPoint - input;

    mITerm += (mKi * error);
    checkLimits(mITerm);

    float dInput = (input - mLastInput);

    float output = mKp * error + mITerm - mKd * dInput;
    checkLimits(output);

    mOutput = output;

    mLastInput = input;
    mLastTime = now;
    return true;
}

void PIDController::setTunings(const float kp, const float ki, const float kd)
{
    if ((kp < 0) || (ki < 0) || (kd < 0)) {
        return;
    }

    mDispKp = kp;
    mDispKi = ki;
    mDispKd = kd;

    auto sampleTimeInSec = ((float)mSampleTime.count()) / 1000;

    mKp = kp;
    mKi = ki * sampleTimeInSec;
    mKd = kd / sampleTimeInSec;

    if (mDirection == ControlDirection::REVERSE) {
        mKp = (0.0 - mKp);
        mKi = (0.0 - mKi);
        mKd = (0.0 - mKd);
    }
}

void PIDController::setSampleTime(const std::chrono::milliseconds newSampleTime)
{
    if (newSampleTime.count() <= 0) {
        return;
    }

    auto ratio = (float)newSampleTime.count() / mSampleTime.count();

    mKi *= ratio;
    mKd /= ratio;
    mSampleTime = newSampleTime;
}

void PIDController::setOutputLimits(const float min, const float max)
{
    if (min >= max) {return; }

    mOutMax = max;
    mOutMin = min;

    if (mMode == ControlMode::AUTOMATIC) {
        checkLimits(mOutput);
        checkLimits(mITerm);
    }
}

void PIDController::setMode(const ControlMode newMode)
{
    if ((newMode == ControlMode::AUTOMATIC) && (mMode == ControlMode::MANUAL)) {
        initalize();
    }
    mMode = newMode;
}

void PIDController::initalize(void)
{
    mITerm = mOutput;
    mLastInput = mInput;
    checkLimits(mITerm);
}

void PIDController::setControllerDirection(const ControlDirection newDirection)
{
    if ((mMode == ControlMode::AUTOMATIC) && (newDirection != mDirection)) {
        mKp = (0.0 - mKp);
        mKi = (0.0 - mKi);
        mKd = (0.0 - mKd);
    }
    mDirection = newDirection;
}

void PIDController::checkLimits(float& value)
{
    if (value > mOutMax) {
        value = mOutMax;
    } else if (value < mOutMin) {
        value = mOutMin;
    }
}

float PIDController::getKp(void) const
{
    return mDispKp;
}
float PIDController::getKi(void) const
{
    return mDispKi;
}
float PIDController::getKd(void) const
{
    return mDispKd;
}

PIDController::ControlDirection PIDController::getDirection(void) const
{
    return mDirection;
}
PIDController::ControlMode PIDController::getMode(void) const
{
    return mMode;
}
