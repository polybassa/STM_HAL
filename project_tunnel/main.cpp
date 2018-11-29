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
#include "Usart.h"
#include "Dma.h"
#include "UsartWithDma.h"

/* DEV LAYER INLCUDES */

/* VIRT LAYER INCLUDES */

/* COM LAYER INCLUDES */

/* APP LAYER INLCUDES */
#include "CanTunnel.h"
#include "CanController.h"
#include "ModemTunnel.h"

/* GLOBAL VARIABLES */
static const int __attribute__((used)) g_DebugZones = ZONE_ERROR | ZONE_WARNING |
                                                      ZONE_VERBOSE | ZONE_INFO;

int main(void)
{
    hal::initFactory<hal::Factory<hal::Gpio> >();
    hal::initFactory<hal::Factory<hal::Usart> >();
    hal::initFactory<hal::Factory<hal::Dma> >();
    hal::initFactory<hal::Factory<hal::UsartWithDma> >();

    static constexpr const bool DEBUG_2_MODEM_TUNNEL = false;

    static constexpr const bool DEBUG_2_CAN_TUNNEL = !DEBUG_2_MODEM_TUNNEL;

    if (DEBUG_2_MODEM_TUNNEL) {
        auto modem = new app::ModemTunnel(hal::Factory<hal::UsartWithDma>::get<hal::Usart::DEBUG_IF>(),
                                          hal::Factory<hal::UsartWithDma>::get<hal::Usart::
                                                                               MODEM_COM>(),
                                          hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_RESET>(),
                                          hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_POWER>(),
                                          hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_SUPPLY>());
    }

    if (DEBUG_2_CAN_TUNNEL) {
        auto can = new app::CanController(hal::Factory<hal::UsartWithDma>::get<hal::Usart::SECCO_COM>(),
                                          hal::Factory<hal::Gpio>::get<hal::Gpio::SECCO_PWR>(),
                                          hal::Factory<hal::Gpio>::getAlternateFunctionGpio<hal::Gpio::USART2_TX>());

        auto tunnel = new app::CanTunnel(hal::Factory<hal::UsartWithDma>::get<hal::Usart::DEBUG_IF>(),
                                         *can);
    }

    os::Task::startScheduler();

    while (1) {}
}

void assert_failed(uint8_t* file, uint32_t line)
{
    Trace(ZONE_ERROR, "ASSERT FAILED: %s:%u", file, line);
}
