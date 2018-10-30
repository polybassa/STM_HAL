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
#include "os_Task.h"
#include "cpp_overrides.h"
#include "trace.h"

/* OS LAYER INCLUDES */
#include "hal_Factory.h"
#include "Gpio.h"
#include "Usart.h"
#include "Dma.h"
#include "UsartWithDma.h"
#include "Spi.h"

/* DEV LAYER INLCUDES */

/* VIRT LAYER INCLUDES */

/* COM LAYER INCLUDES */

/* APP LAYER INLCUDES */
#include "ModemDriver.h"
#include "CanController.h"
#include "CommandMultiplexer.h"
#include "DemoExecuter.h"
#include "Endpoint.h"

/* GLOBAL VARIABLES */
static const int __attribute__((used)) g_DebugZones = ZONE_ERROR | ZONE_WARNING |
                                                      ZONE_VERBOSE | ZONE_INFO;

extern char _version_start;
extern char _version_end;
const std::string VERSION(&_version_start, (&_version_end - &_version_start));

int main(void)
{
    hal::initFactory<hal::Factory<hal::Gpio> >();
    hal::initFactory<hal::Factory<hal::Usart> >();
    hal::initFactory<hal::Factory<hal::Dma> >();
    hal::initFactory<hal::Factory<hal::UsartWithDma> >();
    hal::initFactory<hal::Factory<hal::Spi> >();

    TraceInit();
    Trace(ZONE_INFO, "Version: %s \r\n", VERSION.c_str());

    auto modem = new app::ModemDriver(hal::Factory<hal::UsartWithDma>::get<hal::Usart::
                                                                           MODEM_COM>(),
                                      hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_RESET>(),
                                      hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_POWER>(),
                                      hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_SUPPLY>());

    auto controlsocket = modem->getSocket(app::Socket::Protocol::TCP, ENDPOINT_IP, "62938");
    auto datasocket = modem->getSocket(app::Socket::Protocol::TCP, ENDPOINT_IP, "62979");

    auto can = new app::CanController(hal::Factory<hal::UsartWithDma>::get<hal::Usart::SECCO_COM>(),
                                      hal::Factory<hal::Gpio>::get<hal::Gpio::SECCO_PWR>(),
                                      hal::Factory<hal::Gpio>::getAlternateFunctionGpio<hal::Gpio::USART2_TX>());

    auto demo = new app::DemoExecuter(*can);
    auto __attribute__((used)) mux = new app::CommandMultiplexer(controlsocket, datasocket, *can, *demo);

    os::Task::startScheduler();

    while (1) {}
}

void assert_failed(uint8_t* file, uint32_t line)
{
    Trace(ZONE_ERROR, "ASSERT FAILED: %s:%u", file, line);
}
