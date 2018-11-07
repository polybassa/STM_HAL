// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "Battery.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using dev::Battery;

dev::Battery::Battery(){}

dev::Battery::~Battery(){}

float Battery::getTemperature(void) const
{
    return temperatureSensor.getTemperature();
}

float Battery::getVoltage(void) const
{
    return voltagePeripherie.getValue() * 1 / Battery::VOLTAGE_FACTOR;
}

float Battery::getCurrent(void) const
{
    return (currentPeripherie.getVoltage() - 2.5) * 10;
}

float Battery::getPower(void) const
{
    return getVoltage() * getCurrent();
}
