// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include <cstdint>
#include "interface_Battery.h"

namespace virt
{
class Battery :
    interface ::Battery
{
    float mTemperature = 0.0;
    float mCurrent = 0.0;
    float mVoltage = 0.0;

public:
    Battery(void) = default;
    Battery(const Battery&) = delete;
    Battery(Battery&&) = default;
    Battery& operator=(const Battery&) = delete;
    Battery& operator=(Battery&&) = delete;
    virtual ~Battery(void) {}

    inline virtual float getTemperature(void) const override;
    inline virtual float getVoltage(void) const override;
    inline virtual float getCurrent(void) const override;
    inline virtual float getPower(void) const override;
    inline void setVoltage(const float);
    inline void setCurrent(const float);
    inline void setTemperature(const float);
};
}

float virt::Battery::getTemperature(void) const
{
    return mTemperature;
}
float virt::Battery::getVoltage(void) const
{
    return mVoltage;
}
float virt::Battery::getCurrent(void) const
{
    return mCurrent;
}
float virt::Battery::getPower(void) const
{
    return mVoltage * mCurrent;
}

void virt::Battery::setVoltage(const float voltage)
{
    mVoltage = voltage;
}

void virt::Battery::setCurrent(const float current)
{
    mCurrent = current;
}

void virt::Battery::setTemperature(const float temperature)
{
    mTemperature = temperature;
}
