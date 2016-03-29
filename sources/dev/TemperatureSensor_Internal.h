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

#ifndef SOURCES_PMD_TEMPERATURESENSOR_INTERNAL_H_
#define SOURCES_PMD_TEMPERATURESENSOR_INTERNAL_H_

#include "Adc.h"
#include "interface_TemperatureSensor.h"

namespace dev
{
struct TemperatureSensor_Internal {
    TemperatureSensor_Internal() = delete;
    TemperatureSensor_Internal(const TemperatureSensor_Internal&) = delete;
    TemperatureSensor_Internal(TemperatureSensor_Internal &&) = default;
    TemperatureSensor_Internal& operator=(const TemperatureSensor_Internal&) = delete;
    TemperatureSensor_Internal& operator=(TemperatureSensor_Internal &&) = delete;

    float getTemperature(void) const;
    const enum interface::TemperatureSensor::Description mDescription;

private:

    constexpr TemperatureSensor_Internal(const enum interface::TemperatureSensor::Description desc,
                                         const hal::Adc::Channel&                             peripherie,
                                         const float                                          avgSlope = 0.0,
                                         const float                                          voltageAt25C = 0.0) :
        mDescription(desc), mPeripherie(peripherie),
        mAvgSlope(avgSlope),
        mVoltageAt25C(voltageAt25C) {}

    const hal::Adc::Channel& mPeripherie;
    const float mAvgSlope;              // [mV / ºC]
    const float mVoltageAt25C;          // [V]

    template<typename>
    friend class Factory;
};
}

#endif /* SOURCES_PMD_TEMPERATURESENSOR_INTERNAL_H_ */
