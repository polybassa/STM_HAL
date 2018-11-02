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

#include "TestDRV8302.h"
#include "trace.h"
#include "MotorController.h"
#include "TimSensorBldc.h"
#include "Adc.h"
#include "AdcWithDma.h"
#include <chrono>
#include "medianFilter.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

extern app::MotorController* g_motorCtrl;

os::TaskEndless drv8302Test("drv8302_Test", 2048, os::Task::Priority::MEDIUM, [](const bool&){
                            constexpr const hal::Adc::Channel& poti =
                                hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::NTC_MOTOR>();

                            poti.getValue();
                            os::ThisTask::sleep(std::chrono::milliseconds(5));

                            auto torque = 0.0;

                            while (true) {
                                auto newtorque = poti.getVoltage() - 1.0;

                                newtorque = std::min(1.0, newtorque);
                                newtorque = std::max(-1.0, newtorque);

                                torque -= torque / 20;
                                torque += newtorque / 20;

                                g_motorCtrl->setTorque(torque);

                                os::ThisTask::sleep(std::chrono::milliseconds(5));
                            }
    });
