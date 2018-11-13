// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "Adc.h"
#include "AdcChannel.h"
#include "interface_TemperatureSensor.h"

namespace dev
{
struct TemperatureSensor_Internal {
    TemperatureSensor_Internal() = delete;
    TemperatureSensor_Internal(const TemperatureSensor_Internal&) = delete;
    TemperatureSensor_Internal(TemperatureSensor_Internal&&) = default;
    TemperatureSensor_Internal& operator=(const TemperatureSensor_Internal&) = delete;
    TemperatureSensor_Internal& operator=(TemperatureSensor_Internal&&) = delete;

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
