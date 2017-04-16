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

#ifndef SOURCES_PMD_SLAVECONTROLLER_H_
#define SOURCES_PMD_SLAVECONTROLLER_H_

#include "virtual_BalanceController.h"
#include "virtual_TemperatureSensor.h"
#include "virtual_MotorController.h"
#include "virtual_Battery.h"
#include "virtual_Light.h"
#include "BalanceController.h"
#include "TemperatureSensor.h"
#include "MotorController.h"
#include "Battery.h"
#include "Light.h"
#include "TaskInterruptable.h"
#include <limits>

namespace app
{
class SlaveController final
{
    static constexpr uint32_t STACKSIZE = 2048;
    static constexpr auto UPDATE_INTERNAL_INTERVAL = std::chrono::milliseconds(3);
    static constexpr auto UPDATE_EXTERNAL_INTERVAL = std::chrono::milliseconds(1);

    os::TaskInterruptable mUpdateExternalObjectsTask;
    os::TaskInterruptable mUpdateInternalObjectsTask;

    BalanceController& mRealBalancer;
    const virt::BalanceController& mVirtBalancer;

    const MotorController& mRealMotorController;
    virt::MotorController& mVirtMotorController;

    const dev::Battery& mRealBattery;
    virt::Battery& mVirtBattery;

    const dev::TemperatureSensor_Internal& mRealTempSensor1;
    const dev::TemperatureSensor_NTC& mRealTempSensor2;
    const dev::TemperatureSensor_NTC& mRealTempSensor3;
    const dev::TemperatureSensor_NTC& mRealTempSensor4;

    virt::TemperatureSensor& mVirtTempSensor1;
    virt::TemperatureSensor& mVirtTempSensor2;
    virt::TemperatureSensor& mVirtTempSensor3;
    virt::TemperatureSensor& mVirtTempSensor4;

    const dev::Light& mRealLight1;
    const dev::Light& mRealLight2;

    const virt::Light& mVirtLight1;
    const virt::Light& mVirtLight2;

    void UpdateExternalObjectsTaskFunction(const bool&);
    void UpdateInternalObjectsTaskFunction(const bool&);

public:
    SlaveController(BalanceController &,
                    const virt::BalanceController &,
                    const MotorController &,
                    virt::MotorController &,
                    const dev::Battery &,
                    virt::Battery &,
                    const dev::TemperatureSensor_Internal &,
                    const dev::TemperatureSensor_NTC &,
                    const dev::TemperatureSensor_NTC &,
                    const dev::TemperatureSensor_NTC &,
                    virt::TemperatureSensor &,
                    virt::TemperatureSensor &,
                    virt::TemperatureSensor &,
                    virt::TemperatureSensor &,
                    const dev::Light &,
                    const dev::Light &,
                    const virt::Light &,
                    const virt::Light &);

    SlaveController(const SlaveController &) = delete;
    SlaveController(SlaveController &&) = delete;
    SlaveController& operator=(const SlaveController&) = delete;
    SlaveController& operator=(SlaveController &&) = delete;
};
}

#endif /* SOURCES_PMD_SLAVECONTROLLER_H_ */
