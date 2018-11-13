// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

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
