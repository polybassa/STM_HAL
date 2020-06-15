/* Copyright (C) 2018  Nils Weiss and Henning Mende
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

/* OS LAYER INCLUDES */
#include "os_Task.h"
#include "TaskInterruptable.h"
#include "cpp_overrides.h"
#include "trace.h"

/* HAL LAYER INCLUDES */
#include "hal_Factory.h"
#include "Gpio.h"
#include "Tim.h"
#include "TimPwm.h"
#include "Dma.h"
#include "Usart.h"
#include "UsartWithDma.h"
#include "Spi.h"
#include "SpiWithDma.h"
#include "Exti.h"
#include "Rtc.h"
#include "Adc.h"
#include "AdcChannel.h"
#include "AdcWithDma.h"
#include "CRC.h"
#include "I2c.h"
#include "Nvic.h"
#include "TimInterrupt.h"
#include "FlashAsEeprom.h"

/* DEV LAYER INLCUDES */
#include "DebugInterface.h"

/* VIRT LAYER INCLUDES */

/* COM LAYER INCLUDES */

/* APP LAYER INLCUDES */
#include "LedBlinker.h"

/* GLOBAL VARIABLES */
static const int __attribute__((used)) g_DebugZones = ZONE_ERROR | ZONE_WARNING |
                                                      ZONE_VERBOSE | ZONE_INFO;

extern char _version_start;
extern char _version_end;
const std::string VERSION(&_version_start, (&_version_end - &_version_start));

int main(void)
{
#ifdef ENABLE_CHECK_PIN_CONFIG
    hal::Factory<hal::Gpio>::checkPinConfig();
#endif
    hal::initFactory<hal::Factory<hal::Gpio> >();
    hal::initFactory<hal::Factory<hal::Tim> >();
    hal::initFactory<hal::Factory<hal::Pwm> >();
    hal::initFactory<hal::Factory<hal::Nvic> >();
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
    hal::initFactory<hal::Factory<hal::Crc> >();
    hal::initFactory<hal::Factory<hal::I2c> >();
    hal::initFactory<hal::Factory<hal::FlashAsEeprom> > ();

    TraceInit();
    Trace(ZONE_INFO, "Version: %s \r\n", VERSION.c_str());

    // configure apps
    app::LedBlinker* pLedBlinker = new app::LedBlinker(2);

    // configure user button interrupt
    const hal::Exti& user_interrupt = hal::Factory<hal::Exti>::get<hal::Exti::USER_BUTTON_INT>();
    user_interrupt.registerInterruptCallback([pLedBlinker](void){
        pLedBlinker->changeDirection();
    });
    user_interrupt.enable();

    // create an uart demo task
    new os::TaskInterruptable("uart demo", 2048, os::Task::Priority::MEDIUM, [](const bool& join){
        const hal::UsartWithDma& interface = hal::Factory<hal::UsartWithDma>::get<hal::Usart::DISCO_DEMO_COM>();
        std::array<uint8_t, 4> message_tx = {'?', '?', '\n', '\r'};
        std::array<uint8_t, 2> message_rx;

        while (!join) {
            // wait for two chars
            interface.receive(message_rx);
            // swap these chars
            message_tx.at(0) = message_rx.at(1);
            message_tx.at(1) = message_rx.at(0);
            // send back with additional line break and carriage return
            interface.send(message_tx);
        }
    });

    // create an adc demo task
    new os::TaskInterruptable("adc demo", 2048, os::Task::Priority::MEDIUM, [](const bool& join){
        const hal::Adc::Channel& adc = hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::TEST_ADC>();
        const dev::DebugInterface terminal;
        os::Semaphore adcSemaphore;

        os::ThisTask::sleep(std::chrono::milliseconds(500));

        while (!join) {
            terminal.printf("current adc reading: %f\n\r", adc.getVoltage());
            os::ThisTask::sleep(std::chrono::seconds(1));
        }
    });

    // flash as eeprom demo
    const hal::FlashAsEeprom& counterStorage =
        hal::Factory<hal::FlashAsEeprom>::get<hal::FlashAsEeprom::RESTART_COUNTER>();

    bool status = false;
    uint16_t startCounter = counterStorage.read(status);

    if (status != true) {
        startCounter = 0;
    }
    startCounter++;
    status = counterStorage.write(startCounter);
    Trace(ZONE_INFO, "started %d times (write status: %s)\n", startCounter, status ? "success" : "fail");

    // print info and start scheduler
    Trace(ZONE_INFO, "%d Tasks registered.\n", os::Task::getNumberOfTasks());
    hal::Factory<hal::Gpio>::get<hal::Gpio::LED_BLUE>() = true;
    os::Task::startScheduler();

    while (1) {}
}
