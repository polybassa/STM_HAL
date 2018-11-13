// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include <tuple>
#include "dev_Factory.h"
#include "interface_TemperatureSensor.h"
#include "TemperatureSensor_Internal.h"
#include "TemperatureSensor_NTC.h"

namespace dev
{
struct TemperatureSensor final {
    TemperatureSensor() = delete;
    TemperatureSensor(const TemperatureSensor&) = delete;
    TemperatureSensor(TemperatureSensor&&) = delete;
    TemperatureSensor& operator=(const TemperatureSensor&) = delete;
    TemperatureSensor& operator=(TemperatureSensor&&) = delete;
};

template<>
class Factory<TemperatureSensor>
{
    static constexpr std::tuple<const TemperatureSensor_Internal,
                                const TemperatureSensor_NTC,
                                const TemperatureSensor_NTC,
                                const TemperatureSensor_NTC> Container {
        TemperatureSensor_Internal(interface ::TemperatureSensor::INTERNAL,
                                       hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::INTERNAL_TEMP>(),
                                       4.3,
                                       1.43),
        TemperatureSensor_NTC(interface ::TemperatureSensor::BATTERY,
                                  hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::NTC_BATTERY>()),
        TemperatureSensor_NTC(interface ::TemperatureSensor::MOTOR,
                                  hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::NTC_MOTOR>()),
        TemperatureSensor_NTC(interface ::TemperatureSensor::FET,
                                  hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::NTC_FET>())
    };

public:

    template<interface::TemperatureSensor::Description index>
    static constexpr auto get(void)->std::add_lvalue_reference_t<decltype(std::get<index>(Container))>
    {
        static_assert(static_cast<size_t>(std::get<index>(Container).mDescription) !=
                      static_cast<size_t>(interface ::TemperatureSensor::Description::__ENUM__SIZE),
                      "This is not an Temperaturesensor");
        static_assert(index <= interface ::TemperatureSensor::Description::__ENUM__SIZE,
                          "__ENUM__SIZE is not accessible");
        static_assert(static_cast<size_t>(std::get<index>(
                                                          Container).mDescription) == static_cast<size_t>(index),
                      "Wrong mapping between Description and Container");

        return std::get<index>(Container);
    }
};
}
