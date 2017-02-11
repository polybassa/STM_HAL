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

#include "os_Task.h"
#include "cpp_overrides.h"
#include "trace.h"
#include "SEGGER_SYSVIEW.h"
#include "RealTimeDebugInterface.h"

/* OS LAYER INCLUDES */
#include "hal_Factory.h"
#include "Gpio.h"
#include "Tim.h"
#include "TimHallDecoder.h"
#include "TimHallMeter.h"
#include "TimHalfBridge.h"
#include "TimPwm.h"
#include "Dma.h"
#include "Usart.h"
#include "UsartWithDma.h"
#include "Spi.h"
#include "SpiWithDma.h"
#include "Exti.h"
#include "Rtc.h"
#include "Adc.h"
#include "AdcWithDma.h"
#include "CRC.h"
#include "I2c.h"

/* DEV LAYER INLCUDES */
#include "TimSensorBldc.h"
#include "Battery.h"

/* APP LAYER INLCUDES */
#include "BatteryObserver.h"
#include "DRV8302MotorController.h"

/* GLOBAL VARIABLES */
static const int __attribute__((used)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;
extern char _version_start;
extern char _version_end;

const std::string VERSION(&_version_start, (&_version_end - &_version_start));

app::DRV8302MotorController* g_motorCtrl = nullptr;
dev::RealTimeDebugInterface* g_RTTerminal;

void initializePowerSupply(void)
{
    constexpr auto& supply5V0 = hal::Factory<hal::Gpio>::get<hal::Gpio::ENABLE_5V0_SUPPLY>();
    constexpr auto& supply5V8 = hal::Factory<hal::Gpio>::get<hal::Gpio::ENABLE_5V8_SUPPLY>();

    supply5V0 = true;
    supply5V8 = true;
}

int main(void)
{
    g_RTTerminal = new dev::RealTimeDebugInterface();

    hal::initFactory<hal::Factory<hal::Gpio> >();
    hal::initFactory<hal::Factory<hal::Tim> >();
    hal::initFactory<hal::Factory<hal::HallDecoder> >();
    hal::initFactory<hal::Factory<hal::HallMeter> >();
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
    hal::initFactory<hal::Factory<hal::Adc::Channel> >();
    hal::initFactory<hal::Factory<hal::AdcWithDma> >();
    hal::initFactory<hal::Factory<hal::PhaseCurrentSensor> >();
    hal::initFactory<hal::Factory<hal::Crc> >();
    hal::initFactory<hal::Factory<hal::I2c> >();

    initializePowerSupply();

    TraceInit();
    Trace(ZONE_INFO, "Version: %c \r\n", &_version_start);

    os::ThisTask::sleep(std::chrono::milliseconds(10));
    dev::Battery* mBattery = new dev::Battery();

    g_motorCtrl = new app::DRV8302MotorController( dev::Factory<dev::SensorBLDC>::get<dev::SensorBLDC::BLDC>(), *mBattery, 3.5, 3);

    os::Task::startScheduler();

    while (1) {}
}
