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

#ifndef SOURCES_PMD_BATTERY_H_
#define SOURCES_PMD_BATTERY_H_

#include <cstdint>
#include <type_traits>
#include "Adc.h"
#include "TemperatureSensor.h"
#include "interface_Battery.h"

namespace dev
{
struct Battery final :
    public interface::Battery {
    Battery();
    Battery(const Battery &) = delete;
    Battery(Battery &&) = default;
    Battery& operator=(const Battery&) = delete;
    Battery& operator=(Battery &&) = delete;
    ~Battery() override;

    float getTemperature(void) const override;
    float getVoltage(void) const override;
    float getCurrent(void) const override;
    float getPower(void) const override;

private:
    static constexpr auto& voltagePeripherie = hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::BATTERY_U>();
    static constexpr auto& currentPeripherie = hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::BATTERY_I>();
    static constexpr auto& temperatureSensor =
        dev::Factory<dev::TemperatureSensor>::get<interface::TemperatureSensor::BATTERY>();

    static const uint32_t VALUE_HOLD_TIME_IN_TICKS = 5;
    static constexpr const float VOLTAGE_FACTOR = 15.2;
};
}

#endif /* SOURCES_PMD_BATTERY_H_ */
