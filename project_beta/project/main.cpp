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
#include "Comp.h"

/* DEV LAYER INLCUDES */
#include "TimSensorBldc.h"
#include "Battery.h"

/* VIRT LAYER INCLUDES */
#include "virtual_MotorController.h"
#include "virtual_TemperatureSensor.h"
#include "virtual_Battery.h"
#include "virtual_BalanceController.h"

/* COM LAYER INCLUDES */
#include "DataTransferObject.h"

/* APP LAYER INLCUDES */
#include "BatteryObserver.h"
#include "MotorController.h"
#include "BalanceController.h"
#include "Mpu.h"
#include "SteeringController.h"
#include "SlaveController.h"
#include "BalanceController.h"
#include "Communication.h"

/* GLOBAL VARIABLES */
static const int __attribute__((used)) g_DebugZones = ZONE_ERROR | ZONE_WARNING |
                                                      ZONE_VERBOSE | ZONE_INFO;

extern char _version_start;
extern char _version_end;
const std::string VERSION(&_version_start, (&_version_end - &_version_start));

app::MotorController* g_motorCtrl = nullptr;
app::Mpu* g_Mpu = nullptr;

virt::Light* g_light1 = nullptr;


int main(void)
{
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
    hal::initFactory<hal::Factory<hal::Comp> >();

    TraceInit();
    Trace(ZONE_INFO, "Version: %s \r\n", VERSION.c_str());

    const bool isMaster = hal::Factory<hal::Gpio>::get<hal::Gpio::CONFIG>();

    g_Mpu = new app::Mpu();

    auto battery = new dev::Battery();
    auto vBattery = new virt::Battery();

    g_motorCtrl = new app::MotorController(
                                           dev::Factory<dev::SensorBLDC>::get<dev::SensorBLDC::BLDC>(),
                                           *battery, 200000, 80000);

    auto vMotor = new virt::MotorController();

    auto balancer = new app::BalanceController(*g_Mpu, *g_motorCtrl);
    auto vBalancer = new virt::BalanceController();

    auto virtHeadLight = new virt::Light(interface::Light::HEADLIGHT);

    g_light1 = virtHeadLight;

    auto virtInternalTemp = new virt::TemperatureSensor(interface::TemperatureSensor::INTERNAL);
    auto virtMotorTemp = new virt::TemperatureSensor(interface::TemperatureSensor::MOTOR);
    auto virtBatteryTemp = new virt::TemperatureSensor(interface::TemperatureSensor::BATTERY);
    auto virtFetTemp = new virt::TemperatureSensor(interface::TemperatureSensor::FET);

    static auto masterToSlaveDTO = com::make_dto(*virtHeadLight, *virtHeadLight, *vBalancer);

    static auto slaveToMasterDTO = com::make_dto(*virtMotorTemp,
                                          *virtBatteryTemp,
                                          *virtFetTemp,
                                          *vBattery,
                                          *vMotor);


	// TODO make different build targets to reduce flash size
    if (!isMaster) {
    	[[gnu::used]] auto comApp =
            new app::Communication<decltype(masterToSlaveDTO), decltype(slaveToMasterDTO)>(
                                                                                           hal::Factory<hal::
                                                                                                        UsartWithDma>::
                                                                                           get<hal::Usart::MSCOM_IF>(),
                                                                                           masterToSlaveDTO,
                                                                                           slaveToMasterDTO);
    } else {
    	[[gnu::used]] auto comApp =
            new app::Communication<decltype(slaveToMasterDTO), decltype(masterToSlaveDTO)>(
                                                                                           hal::Factory<hal::
                                                                                                        UsartWithDma>::
                                                                                           get<hal::Usart::MSCOM_IF>(),
                                                                                           slaveToMasterDTO,
                                                                                           masterToSlaveDTO);
    }

    if (!isMaster) {
        //===================== APPS in Slave ==========================
    	[[gnu::unused]] auto slaveController = new app::SlaveController(
                                                        *balancer,
                                                        *vBalancer,
                                                        *g_motorCtrl,
                                                        *vMotor,
                                                        *battery,
                                                        *vBattery,
                                                        dev::Factory<dev::TemperatureSensor>::get<interface::
                                                                                                  TemperatureSensor::
                                                                                                  INTERNAL>(),
                                                        dev::Factory<dev::TemperatureSensor>::get<interface::
                                                                                                  TemperatureSensor::
                                                                                                  MOTOR>(),
                                                        dev::Factory<dev::TemperatureSensor>::get<interface::
                                                                                                  TemperatureSensor::
                                                                                                  FET>(),
                                                        dev::Factory<dev::TemperatureSensor>::get<interface::
                                                                                                  TemperatureSensor::
                                                                                                  BATTERY>(),
                                                        *virtInternalTemp,
                                                        *virtMotorTemp,
                                                        *virtFetTemp,
                                                        *virtBatteryTemp,
                                                        dev::Factory<dev::Light>::get<interface::Light::HEADLIGHT>(),
                                                        dev::Factory<dev::Light>::get<interface::Light::HEADLIGHT>(),
                                                        *virtHeadLight,
                                                        *virtHeadLight);
    }
    else {
        //===================== APPS in Master ==========================
        constexpr const auto& straingaugeSensor =
            dev::Factory<dev::StraingaugeSensor>::get<dev::StraingaugeSensor::STRAINGAUGESENSOR>();
        [[gnu::unused]] auto steering = new app::SteeringController(*balancer, *vBalancer, straingaugeSensor);
    }

    os::Task::startScheduler();

    while (1) {}
}
