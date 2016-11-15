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

#ifndef SOURCES_PMD_TEMPERATURESENSOR_NTC_H_
#define SOURCES_PMD_TEMPERATURESENSOR_NTC_H_

#include <cstdint>
#include "Adc.h"
#include "AdcChannel.h"
#include "interface_TemperatureSensor.h"
#include <array>
#include <utility>

namespace dev
{
struct TemperatureSensor_NTC {
    TemperatureSensor_NTC() = delete;
    TemperatureSensor_NTC(const TemperatureSensor_NTC&) = delete;
    TemperatureSensor_NTC(TemperatureSensor_NTC &&) = default;
    TemperatureSensor_NTC& operator=(const TemperatureSensor_NTC&) = delete;
    TemperatureSensor_NTC& operator=(TemperatureSensor_NTC &&) = delete;

    float getTemperature(void) const;
    const enum interface::TemperatureSensor::Description mDescription;

private:
    constexpr TemperatureSensor_NTC(const enum interface::TemperatureSensor::Description& desc,
                                    const hal::Adc::Channel&                              peripherie) :
        mDescription(desc),
        mPeripherie(peripherie) {}

    const hal::Adc::Channel& mPeripherie;

    static constexpr uint8_t LookupTableSize = 19;
    static const std::array<std::pair<float, int8_t>, LookupTableSize> TemperaturesLookupTable;

    template<typename>
    friend class Factory;
};
}

#endif /* SOURCES_PMD_TEMPERATURESENSOR_NTC_H_ */
