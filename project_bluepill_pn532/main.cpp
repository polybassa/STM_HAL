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

/* HAL LAYER INCLUDES */
#include "hal_Factory.h"
#include "Gpio.h"
#include "Adc.h"
#include "Exti.h"
#include "Usb.h"
#include "Tim.h"
#include "Spi.h"

/* DEV LAYER INLCUDES */

/* VIRT LAYER INCLUDES */

/* COM LAYER INCLUDES */

/* APP LAYER INLCUDES */
#include "Horrorscope.h"

/* GLOBAL VARIABLES */
static const int __attribute__((used)) g_DebugZones = ZONE_ERROR | ZONE_WARNING |
                                                      ZONE_VERBOSE | ZONE_INFO;

extern char _version_start;
extern char _version_end;
const std::string VERSION(&_version_start, (&_version_end - &_version_start));

int main(void)
{
    TraceInit();
    Trace(ZONE_INFO, "Version: %s \r\n", VERSION.c_str());

    hal::initFactory<hal::Factory<hal::Gpio> >();
    //hal::initFactory<hal::Factory<hal::Usb> >();
    hal::initFactory<hal::Factory<hal::Spi> > ();

    os::Task::startScheduler();
    Trace(ZONE_ERROR, "This shouldn't happen!\r\n");
    configASSERT(0);
}

void assert_failed(uint8_t* file, uint32_t line)
{
    Trace(ZONE_ERROR, "ASSERT FAILED: %s:%u", file, line);
}
