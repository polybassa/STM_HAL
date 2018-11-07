// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

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
