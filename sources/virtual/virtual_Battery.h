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

#ifndef SOURCES_PMD_VIRTUAL_BATTERY_H_
#define SOURCES_PMD_VIRTUAL_BATTERY_H_

#include <cstdint>
#include "interface_Battery.h"

namespace virt
{
class Battery :
    interface::Battery
{
    float mTemperature = 0.0;
    float mCurrent = 0.0;
    float mVoltage = 0.0;

public:
    Battery(void) = default;
    Battery(const Battery&) = delete;
    Battery(Battery &&) = default;
    Battery& operator=(const Battery&) = delete;
    Battery& operator=(Battery &&) = delete;
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

#endif /* SOURCES_PMD_VIRTUAL_BATTERY_H_ */
