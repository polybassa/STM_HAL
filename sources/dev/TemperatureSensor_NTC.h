// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

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
    TemperatureSensor_NTC(TemperatureSensor_NTC&&) = default;
    TemperatureSensor_NTC& operator=(const TemperatureSensor_NTC&) = delete;
    TemperatureSensor_NTC& operator=(TemperatureSensor_NTC&&) = delete;

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
