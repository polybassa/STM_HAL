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

#include "SteeringController.h"
#include "trace.h"
#include <cmath>

using app::SteeringController;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

SteeringController::SteeringController(
                                       app::BalanceController&       masterBalance,
                                       virt::BalanceController&      slaveBalance,
                                       const dev::StraingaugeSensor& dms) :
    os::DeepSleepModule(),
    mSteeringControllerTask("8SteeringControl",
                            SteeringController::STACKSIZE,
                            os::Task::Priority::VERY_HIGH,
                            [this](const bool& join)
                            {
                                SteeringControllerTaskFunction(join);
                            }),
    mStraingaugeUpdateTask("9StrainGaugeUpdate", 1024, os::Task::Priority::VERY_LOW, [this](const bool& join){
                               StraingaugeUpdateTaskFunction(join);
                           }),
    mQueue(),
    mMasterBalance(masterBalance),
    mSlaveBalance(slaveBalance),
    mDms(dms)
{}

void SteeringController::enterDeepSleep(void)
{
    mSteeringControllerTask.join();
    mStraingaugeUpdateTask.join();
}

void SteeringController::exitDeepSleep(void)
{
    mStraingaugeUpdateTask.start();
    mSteeringControllerTask.start();
}

// Do all steering related stuff here, and use the balanceControllers as output
void SteeringController::SteeringControllerTaskFunction(const bool& join)
{
    do {
        std::pair<float, float> newDirectionAndSpeed;
        mQueue.receive(newDirectionAndSpeed, 100);

        float speed = newDirectionAndSpeed.second;
        float direction = newDirectionAndSpeed.first;

        //Trace(ZONE_INFO, "Received direction %f  speed %f\r\n", direction, speed);

        mMasterBalance.setTargetAngleInDegree(speed + direction);
        mSlaveBalance.setTargetAngleInDegree(0.0 - speed + direction);
    } while (!join);
}

void SteeringController::StraingaugeUpdateTaskFunction(const bool& join)
{
    do {
        auto value = mDms.getDirection();
        value = 0.0; // TODO REMOVE
        mStraingaugeValueQueue.overwrite(value);
        os::ThisTask::sleep(std::chrono::milliseconds(30));
    } while (!join);
}

void SteeringController::setDirectionAndAngleInDegree(const float direction, const float speed)
{
    mQueue.overwrite(std::make_pair(direction, speed));
}

void SteeringController::setDirectionAndAngleInDegree(const std::pair<float, float> directionAndSpeed)
{
    mQueue.overwrite(directionAndSpeed);
}

void SteeringController::setAngleInDegree(const float speed)
{
    float direction = 0.0;
    mStraingaugeValueQueue.peek(direction, 0);
    setDirectionAndAngleInDegree(direction, speed);
}
