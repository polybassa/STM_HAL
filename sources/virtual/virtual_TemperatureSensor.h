// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include <cstdint>
#include "interface_TemperatureSensor.h"

namespace virt
{
struct TemperatureSensor {
    TemperatureSensor() = delete;
    TemperatureSensor(const TemperatureSensor&) = delete;
    TemperatureSensor(TemperatureSensor&&) = default;
    TemperatureSensor& operator=(const TemperatureSensor&) = delete;
    TemperatureSensor& operator=(TemperatureSensor&&) = delete;

    inline void setTemperature(const float);
    inline float getTemperature(void) const;

    const enum interface::TemperatureSensor::Description mDescription;

    explicit TemperatureSensor(const enum interface::TemperatureSensor::Description& desc) :
        mDescription(desc) {}

private:
    float mTemperature = 0.0;
};
}

float virt::TemperatureSensor::getTemperature(void) const
{
    return mTemperature;
}

void virt::TemperatureSensor::setTemperature(const float temperature)
{
    mTemperature = temperature;
}
