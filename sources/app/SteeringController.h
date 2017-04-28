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

#ifndef SOURCES_PMD_STEERINGCONTROLLER_H_
#define SOURCES_PMD_STEERINGCONTROLLER_H_

#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "os_Queue.h"
#include "virtual_BalanceController.h"
#include "BalanceController.h"
#include "StraingaugeSensor.h"
#include <limits>

namespace app
{
class SteeringController final :
    private os::DeepSleepModule
{
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr uint32_t STACKSIZE = 1024;

    os::TaskInterruptable mSteeringControllerTask;
    os::TaskInterruptable mStraingaugeUpdateTask;

    os::Queue<std::pair<float, float>, 1> mQueue;
    os::Queue<float, 1> mStraingaugeValueQueue;

    BalanceController& mMasterBalance;
    virt::BalanceController& mSlaveBalance;
    const dev::StraingaugeSensor& mDms;

    void SteeringControllerTaskFunction(const bool&);
    void StraingaugeUpdateTaskFunction(const bool&);

public:
    SteeringController(
                       BalanceController & masterBalance,
                       virt::BalanceController & slaveBalance,
                       const dev::StraingaugeSensor & dms);

    SteeringController(const SteeringController &) = delete;
    SteeringController(SteeringController &&) = delete;
    SteeringController& operator=(const SteeringController&) = delete;
    SteeringController& operator=(SteeringController &&) = delete;

    void setDirectionAndAngleInDegree(const float, const float);
    void setDirectionAndAngleInDegree(const std::pair<float, float> );
    void setAngleInDegree(const float);
};
}

#endif /* SOURCES_PMD_STEERINGCONTROLLER_H_ */
