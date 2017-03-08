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

#include "TemperatureSensor_NTC.h"
#include "trace.h"
#include <cmath>

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

const std::array<std::pair<float, int8_t>,
                 dev::TemperatureSensor_NTC::LookupTableSize> dev::TemperatureSensor_NTC::TemperaturesLookupTable =
{{
     {3914, -20}, {3861, -10},
     {3733, 0}, {3637, 10},
     {3310, 25}, {2439, 50},
     {2035, 60}, {1647, 70},
     {1301, 80}, {1011, 90},
     {777, 100}, {454, 120}
 }};

float dev::TemperatureSensor_NTC::getTemperature(void) const
{
    uint8_t indexStart = 0;
    uint8_t indexEnd = TemperaturesLookupTable.size() - 1;
    uint8_t middle = 0;
    float actualValue = mPeripherie.getValue();

    do {
        middle = (indexEnd + indexStart) / 2;

        if (TemperaturesLookupTable[middle].first > actualValue) {
            indexStart = middle;
        } else {
            indexEnd = middle;
        }
    } while ((indexEnd - indexStart) > 1);

    const auto& startPair = TemperaturesLookupTable[indexStart];
    const auto& endPair = TemperaturesLookupTable[indexEnd];

    const float startVoltage = startPair.first;
    const float endVoltage = endPair.first;

    const float startTemperature = static_cast<float>(startPair.second);
    const float endTemperature = static_cast<float>(endPair.second);

    const float percRange = (actualValue - endVoltage) / (startVoltage - endVoltage);
    return (startTemperature - endTemperature) * percRange + endTemperature;
}
