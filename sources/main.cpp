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

/* GENERAL INCLUDES */
#include <string>
#include "cpp_overrides.h"
#include "trace.h"

/* OS LAYER INCLUDES */
#include "pmd_Task.h"

/* HAL LAYER INCLUDES */
#include "pmd_hal_Factory.h"
#include "pmd_Gpio.h"
#include "pmd_Tim.h"
#include "pmd_TimHallDecoder.h"
#include "pmd_TimHalfBridge.h"
#include "pmd_TimPwm.h"
#include "pmd_Dma.h"
#include "pmd_Usart.h"
#include "pmd_UsartWithDma.h"
#include "pmd_Spi.h"
#include "pmd_SpiWithDma.h"
#include "pmd_Exti.h"
#include "pmd_Rtc.h"
#include "pmd_Adc.h"
#include "pmd_CRC.h"
#include "pmd_I2c.h"

/* DEV LAYER INLCUDES */
#include "pmd_TimSensorBldc.h"

/* APP LAYER INLCUDES */
#include "pmd_BatteryObserver.h"
#include "pmd_MotorController.h"

/* GLOBAL VARIABLES */
static const int __attribute__((used)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;
extern char _version_start;
extern char _version_end;
const std::string VERSION(&_version_start, (&_version_end - &_version_start));

void initializePowerSupply(void)
{
    constexpr auto& supply5V0 = hal::Factory<hal::Gpio>::get<hal::Gpio::ENABLE_5V0_SUPPLY>();
    constexpr auto& supply5V8 = hal::Factory<hal::Gpio>::get<hal::Gpio::ENABLE_5V8_SUPPLY>();
    constexpr auto& nSupplyWifi = hal::Factory<hal::Gpio>::get<hal::Gpio::NWP_SUPPLY_N_ENABLE>();

    supply5V0 = true;
    supply5V8 = true;
    nSupplyWifi = true; // Turn Wifi Off
}

int main(void)
{
    hal::initFactory<hal::Factory<hal::Gpio> >();
    hal::initFactory<hal::Factory<hal::Tim> >();
    hal::initFactory<hal::Factory<hal::HallDecoder> >();
    hal::initFactory<hal::Factory<hal::HalfBridge> >();
    hal::initFactory<hal::Factory<hal::Pwm> >();
    hal::initFactory<hal::Factory<hal::Exti> >();
    hal::initFactory<hal::Factory<hal::Dma> >();
    hal::initFactory<hal::Factory<hal::Usart> >();
    hal::initFactory<hal::Factory<hal::UsartWithDma> >();
    hal::initFactory<hal::Factory<hal::Spi> >();
    hal::initFactory<hal::Factory<hal::SpiWithDma> >();
    hal::initFactory<hal::Factory<hal::Rtc> >();
    hal::initFactory<hal::Factory<hal::Adc> >();
    hal::initFactory<hal::Factory<hal::Crc> >();
    hal::initFactory<hal::Factory<hal::I2c> >();

    initializePowerSupply();

    TraceInit();
    Trace(ZONE_INFO, "Version: %c \r\n", &_version_start);

    os::ThisTask::sleep(std::chrono::milliseconds(10));

     auto motor = new app::MotorController(
        dev::Factory<dev::SensorBLDC>::get<dev::SensorBLDC::BLDC>(), 0.00768, 0.844, 0.8, 0.5);

    os::Task::startScheduler();

    while (1) {}
}
