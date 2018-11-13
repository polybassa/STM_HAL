// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include <cstdint>
#include <type_traits>
#include "Adc.h"
#include "TemperatureSensor.h"
#include "interface_Battery.h"

namespace dev
{
struct Battery final :
    public interface ::Battery {
#include "Battery_config.h"

    Battery();
    Battery(const Battery&) = delete;
    Battery(Battery&&) = default;
    Battery& operator=(const Battery&) = delete;
    Battery& operator=(Battery&&) = delete;
    ~Battery() override;

    float getTemperature(void) const override;
    float getVoltage(void) const override;
    float getCurrent(void) const override;
    float getPower(void) const override;

private:
    static constexpr const hal::Adc::Channel& voltagePeripherie =
        hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::BATTERY_U>();
    static constexpr const hal::Adc::Channel& currentPeripherie =
        hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::BATTERY_I>();
    static constexpr auto& temperatureSensor =
        dev::Factory<dev::TemperatureSensor>::get<interface ::TemperatureSensor::BATTERY>();
};
}
