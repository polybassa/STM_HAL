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
#include "DRV8302MotorController.h"
#include "TimSensorBldc.h"
#include "RealTimeDebugInterface.h"
#include "Adc.h"
#include "AdcWithDma.h"
#include <chrono>
#include "medianFilter.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

extern app::DRV8302MotorController* g_motorCtrl;
extern dev::RealTimeDebugInterface* g_RTTerminal;

os::TaskEndless drv8302Test("drv8302_Test", 2048, os::Task::Priority::MEDIUM, [] (const bool&){
                                g_RTTerminal->printStartupMessage();

                                constexpr const hal::Adc::Channel& poti =
                                    hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::NTC_MOTOR>();

                                constexpr const auto& motor = dev::Factory<dev::SensorBLDC>::get<dev::SensorBLDC::BLDC>();

                                poti.getValue();
                                os::ThisTask::sleep(std::chrono::milliseconds(5));

                                bool torque = false;

                                while (true) {
//                                    if ((poti.getValue() > 3000) && (torque == false)) {
//                                        torque = true;
//                                        g_motorCtrl->setTorque(0.25);
//                                        g_RTTerminal->printf("New torque: %4d\n", static_cast<int32_t>(250));
//                                    } else if ((poti.getValue() <= 3000) && (torque == true)) {
//                                        torque = false;
//                                        g_motorCtrl->setTorque(0);
//                                        g_RTTerminal->printf("New torque: %4d\n”", static_cast<int32_t>(0));
//                                    } else {
//                                        torque = -0.15;
//                                        g_motorCtrl->setTorque(-0.15);
//                                    }

//                                    auto torque = (poti.getValue() / 2000.0) - 1.0;
//                                    g_motorCtrl->setTorque(torque);

                                    g_motorCtrl->setTorque(poti.getValue() / 4);

                                    os::ThisTask::sleep(std::chrono::milliseconds(5));
                                }
                            });
