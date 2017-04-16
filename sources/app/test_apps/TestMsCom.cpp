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

#include "TestMsCom.h"
#include "DebugInterface.h"
#include "Gpio.h"
#include "virtual_Light.h"
#include "Communication.h"
#include "Adc.h"

using os::TaskEndless;

extern app::Communication* g_Com;

const TaskEndless app::testMsCom("MsCom", 2048, os::Task::Priority::MEDIUM, [] (const bool&){
                                     const dev::DebugInterface com;

                                     constexpr auto& master = hal::Factory<hal::Gpio>::get<hal::Gpio::CONFIG>();

                                     virt::Light headLight(virt::Light::HEADLIGHT, *g_Com);
                                     virt::Light backLight(virt::Light::BACKLIGHT, *g_Com);

                                     com.print("Start MS Com test");

                                     while (true) {
                                         if (!master) {
                                             auto color = headLight.getColor();
//            com.print("R:%d G:%d B:%d\r\n", color.red, color.green, color.blue);
                                             color = backLight.getColor();
//            com.print("R:%d G:%d B:%d\r\n", color.red, color.green, color.blue);
                                             os::ThisTask::sleep(std::chrono::milliseconds(10));
                                         } else {
                                             static uint8_t value = 0;

                                             headLight.setColor({value, value, value});
                                             backLight.setColor({value++, 0, 0});

//            constexpr auto& tBatt = virt::Factory<virt::TemperatureSensor>::get<virt::TemperatureSensor::BATTERY>();
//            constexpr auto& tMotor = virt::Factory<virt::TemperatureSensor>::get<virt::TemperatureSensor::MOTOR>();
//            constexpr auto& tFet = virt::Factory<virt::TemperatureSensor>::get<virt::TemperatureSensor::FET>();
//
//            virt::Battery batt;
//            virt::MotorController motor;
//
//            com.print("Slave values\r\n");
//            com.print("Temperature Battery: %f\r\n", tBatt.getTemperature());
//            com.print("Temperature Motor: %f\r\n", tMotor.getTemperature());
//            com.print("Temperature FET: %f\r\n", tFet.getTemperature());
//            com.print("Battery voltage: %f\r\n", batt.getVoltage());
//            com.print("Battery current: %f\r\n", batt.getCurrent());
//            com.print("Motor RPS: %f\r\n", motor.getCurrentRPS());

                                             os::ThisTask::sleep(std::chrono::milliseconds(10));
                                         }
                                     }
                                 });
