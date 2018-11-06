/* Copyright (C) 2015  Nils Weiss
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

#include "interface_MotorController.h"

namespace virt
{
class MotorController final :
    public interface ::MotorController
{
    float mTorque = 0.0;
    float mRps = 0.0;

public:
    MotorController() {}
    MotorController(const MotorController&) = delete;
    MotorController(MotorController&&) = delete;
    MotorController& operator=(const MotorController&) = delete;
    MotorController& operator=(MotorController&&) = delete;

    inline float getTorque(void) const;
    inline virtual void setTorque(const float) override;
    inline virtual float getCurrentRPS(void) const override;
    inline void setCurrentRPS(const float);
};
}

void virt::MotorController::setTorque(const float torque)
{
    mTorque = torque;
}

float virt::MotorController::getTorque(void) const
{
    return mTorque;
}

void virt::MotorController::setCurrentRPS(const float rps)
{
    mRps = rps;
}

float virt::MotorController::getCurrentRPS(void) const
{
    return mRps;
}
