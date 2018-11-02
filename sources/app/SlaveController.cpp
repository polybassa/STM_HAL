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

#include "SlaveController.h"
#include "trace.h"
#include <cmath>

using app::SlaveController;

static const int __attribute__((used)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

constexpr std::chrono::milliseconds SlaveController:: UPDATE_INTERNAL_INTERVAL;
constexpr std::chrono::milliseconds SlaveController::UPDATE_EXTERNAL_INTERVAL;

SlaveController::SlaveController(
                                 BalanceController&                     rBal,
                                 const virt::BalanceController&         vBal,
                                 const MotorController&                 rMot,
                                 virt::MotorController&                 vMot,
                                 const dev::Battery&                    rBat,
                                 virt::Battery&                         vBat,
                                 const dev::TemperatureSensor_Internal& rTemp1,
                                 const dev::TemperatureSensor_NTC&      rTemp2,
                                 const dev::TemperatureSensor_NTC&      rTemp3,
                                 const dev::TemperatureSensor_NTC&      rTemp4,
                                 virt::TemperatureSensor&               vTemp1,
                                 virt::TemperatureSensor&               vTemp2,
                                 virt::TemperatureSensor&               vTemp3,
                                 virt::TemperatureSensor&               vTemp4,
                                 const dev::Light&                      rLight1,
                                 const dev::Light&                      rLight2,
                                 const virt::Light&                     vLight1,
                                 const virt::Light&                     vLight2) :
    mUpdateExternalObjectsTask("6UpdateExternalTask",
                               SlaveController::STACKSIZE,
                               os::Task::Priority::HIGH,
                               [this](const bool& join)
{
    UpdateExternalObjectsTaskFunction(join);
}),
    mUpdateInternalObjectsTask("7UpdateInternalTask",
                               SlaveController::STACKSIZE,
                               os::Task::Priority::HIGH,
                               [this](const bool& join)
{
    UpdateInternalObjectsTaskFunction(join);
}),
    mRealBalancer(rBal),
    mVirtBalancer(vBal),
    mRealMotorController(rMot),
    mVirtMotorController(vMot),
    mRealBattery(rBat),
    mVirtBattery(vBat),
    mRealTempSensor1(rTemp1),
    mRealTempSensor2(rTemp2),
    mRealTempSensor3(rTemp3),
    mRealTempSensor4(rTemp4),
    mVirtTempSensor1(vTemp1),
    mVirtTempSensor2(vTemp2),
    mVirtTempSensor3(vTemp3),
    mVirtTempSensor4(vTemp4),
    mRealLight1(rLight1),
    mRealLight2(rLight2),
    mVirtLight1(vLight1),
    mVirtLight2(vLight2)
{}

void SlaveController::UpdateInternalObjectsTaskFunction(const bool& join)
{
    do {
        mRealBalancer.setTargetAngleInDegree(mVirtBalancer.getTargetAngleInDegree());

        mRealLight1.setColor(mVirtLight1.getColor());
        mRealLight2.setColor(mVirtLight2.getColor());

        os::ThisTask::sleep(UPDATE_INTERNAL_INTERVAL);
    } while (!join);
}

void SlaveController::UpdateExternalObjectsTaskFunction(const bool& join)
{
    do {
        mVirtMotorController.setCurrentRPS(mRealMotorController.getCurrentRPS());

        mVirtBattery.setCurrent(mRealBattery.getCurrent());
        mVirtBattery.setVoltage(mRealBattery.getVoltage());
        mVirtBattery.setTemperature(mRealBattery.getTemperature());

        mVirtTempSensor1.setTemperature(mRealTempSensor1.getTemperature());
        mVirtTempSensor2.setTemperature(mRealTempSensor2.getTemperature());
        mVirtTempSensor3.setTemperature(mRealTempSensor3.getTemperature());
        mVirtTempSensor4.setTemperature(mRealTempSensor4.getTemperature());

        os::ThisTask::sleep(UPDATE_EXTERNAL_INTERVAL);
    } while (!join);
}
