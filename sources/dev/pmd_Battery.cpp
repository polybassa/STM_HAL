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

#include "pmd_Battery.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using dev::Battery;

float Battery::getTemperature(void) const
{
    return temperatureSensor.getTemperature();
}

float Battery::getVoltage(void) const
{
    return voltagePeripherie.getVoltage() * Battery::VOLTAGE_FACTOR;
}

float Battery::getCurrent(void) const
{
    return (currentPeripherie.getVoltage() - 2.5) * 10;
}

float Battery::getPower(void) const
{
    return getVoltage() * getCurrent();
}
