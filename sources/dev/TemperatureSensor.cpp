// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "TemperatureSensor.h"

constexpr std::tuple<const dev::TemperatureSensor_Internal,
                     const dev::TemperatureSensor_NTC,
                     const dev::TemperatureSensor_NTC,
                     const dev::TemperatureSensor_NTC> dev::Factory<dev::TemperatureSensor>::Container;
