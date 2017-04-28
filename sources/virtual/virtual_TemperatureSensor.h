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

#ifndef SOURCES_PMD_VIRTUAL_TEMPERATURESENSOR_H_
#define SOURCES_PMD_VIRTUAL_TEMPERATURESENSOR_H_

#include <cstdint>
#include "interface_TemperatureSensor.h"

namespace virt
{
struct TemperatureSensor {
    TemperatureSensor() = delete;
    TemperatureSensor(const TemperatureSensor&) = delete;
    TemperatureSensor(TemperatureSensor &&) = default;
    TemperatureSensor& operator=(const TemperatureSensor&) = delete;
    TemperatureSensor& operator=(TemperatureSensor &&) = delete;

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

#endif /* SOURCES_PMD_VIRTUAL_TEMPERATURESENSOR_H_ */
