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

#ifndef SOURCES_PMD_VIRTUAL_BALANCECONTROLLER_H_
#define SOURCES_PMD_VIRTUAL_BALANCECONTROLLER_H_

#include "interface_BalanceController.h"

namespace virt
{
class BalanceController final :
    public interface::BalanceController
{
    float mTargetAngle = 0.0;
public:
    BalanceController() {}
    BalanceController(const BalanceController &) = delete;
    BalanceController(BalanceController &&) = delete;
    BalanceController& operator=(const BalanceController&) = delete;
    BalanceController& operator=(BalanceController &&) = delete;
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
#endif /* SOURCES_PMD_VIRTUAL_MOTORCONTROLLER_H_ */
