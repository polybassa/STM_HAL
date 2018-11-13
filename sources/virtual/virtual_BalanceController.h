// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "interface_BalanceController.h"

namespace virt
{
class BalanceController final :
    public interface ::BalanceController
{
    float mTargetAngle = 0.0;
public:
    BalanceController() {}
    BalanceController(const BalanceController&) = delete;
    BalanceController(BalanceController&&) = delete;
    BalanceController& operator=(const BalanceController&) = delete;
    BalanceController& operator=(BalanceController&&) = delete;
    virtual ~BalanceController(){}

    inline virtual void setTargetAngleInDegree(const float angle) override;
    inline float getTargetAngleInDegree(void) const;
};
}

void virt::BalanceController::setTargetAngleInDegree(const float angle)
{
    mTargetAngle = angle;
}

float virt::BalanceController::getTargetAngleInDegree(void) const
{
    return mTargetAngle;
}
