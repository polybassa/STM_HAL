/* Copyright (C) 2015  Nils Weiss, Alexander Strobl
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

#include "TemperatureSensor_Internal.h"
#include "trace.h"
#include <cmath>

static const int __attribute__((unused)) g_DebugZones = 0; // ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

float dev::TemperatureSensor_Internal::getTemperature(void) const
{
    /* Calibration values of this chip are stored at these memory addresses. One for 30 degree, one for 110 degree celsius */
    static const uint32_t TS_CAL1 = 0x1ffff7b8;
    static const uint32_t TS_CAL2 = 0x1ffff7c2;

    uint16_t const* const adcValue30 = reinterpret_cast<uint16_t const* const>(TS_CAL1);
    uint16_t const* const adcValue110 = reinterpret_cast<uint16_t const* const>(TS_CAL2);

    auto getVoltageFromAdcValue = [] (const uint16_t value){
        return (3.3 / std::pow(2, 12)) * value;
    };

    auto voltage30 = getVoltageFromAdcValue(*adcValue30);
    auto voltage110 = getVoltageFromAdcValue(*adcValue110);
    auto slope = ((voltage110 - voltage30) / 80);

    auto voltageNow = mPeripherie.getVoltage();

    auto retValue = (voltageNow - voltage30) / slope + 60; // Don't know if this formula is correct

    Trace(ZONE_INFO, "(%f - %f) / %f + 60 = %f\r\n", voltageNow, voltage30, slope, retValue);
    return retValue;
}
