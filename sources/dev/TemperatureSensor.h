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

#ifndef SOURCES_PMD_TEMPERATURESENSOR_H_
#define SOURCES_PMD_TEMPERATURESENSOR_H_

#include <tuple>
#include "dev_Factory.h"
#include "interface_TemperatureSensor.h"
#include "TemperatureSensor_Internal.h"
#include "TemperatureSensor_NTC.h"

namespace dev
{
struct TemperatureSensor final {
    TemperatureSensor() = delete;
    TemperatureSensor(const TemperatureSensor &) = delete;
    TemperatureSensor(TemperatureSensor &&) = delete;
    TemperatureSensor& operator=(const TemperatureSensor&) = delete;
    TemperatureSensor& operator=(TemperatureSensor &&) = delete;
};

template<>
class Factory<TemperatureSensor>
{
    static constexpr std::tuple<const TemperatureSensor_Internal,
                                const TemperatureSensor_NTC,
                                const TemperatureSensor_NTC,
                                const TemperatureSensor_NTC> Container {
        TemperatureSensor_Internal(interface::TemperatureSensor::INTERNAL,
                                   hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::INTERNAL_TEMP>(),
                                   4.3,
                                   1.43),
        TemperatureSensor_NTC(interface::TemperatureSensor::BATTERY,
                              hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::NTC_BATTERY>()),
        TemperatureSensor_NTC(interface::TemperatureSensor::MOTOR,
                              hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::NTC_MOTOR>()),
        TemperatureSensor_NTC(interface::TemperatureSensor::FET,
                              hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::NTC_FET>())
    };

public:

    template<interface::TemperatureSensor::Description index>
    static constexpr auto get(void)->std::add_lvalue_reference_t<decltype(std::get<index>(Container))>
    {
        static_assert(static_cast<size_t>(std::get<index>(Container).mDescription) !=
                      static_cast<size_t>(interface::TemperatureSensor::Description::__ENUM__SIZE),
                      "This is not an Temperaturesensor");
        static_assert(index <= interface::TemperatureSensor::Description::__ENUM__SIZE,
                      "__ENUM__SIZE is not accessible");
        static_assert(static_cast<size_t>(std::get<index>(
                                                          Container).mDescription) == static_cast<size_t>(index),
                      "Wrong mapping between Description and Container");

        return std::get<index>(Container);
    }
};
}

#endif /* SOURCES_PMD_TEMPERATURESENSOR_NTC_H_ */
