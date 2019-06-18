// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_PHASECURRENTSENSOR_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_PHASECURRENTSENSOR_CONFIG_DESCRIPTION_H_

enum Description {
    __ENUM__SIZE
};

static constexpr const uint32_t MAX_NUMBER_OF_MEASUREMENTS = 64;
static constexpr const float SHUNT_RESISTANCE = 0.002; // Ohm
static constexpr const float MEASUREMENT_GAIN = 22.1;
static constexpr const float FILTERWIDTH = 128;

#else
#ifndef SOURCES_PMD_PHASECURRENTSENSOR_CONFIG_CONTAINER_H_
#define SOURCES_PMD_PHASECURRENTSENSOR_CONFIG_CONTAINER_H_

static constexpr std::array<const PhaseCurrentSensor, PhaseCurrentSensor::__ENUM__SIZE> Container =
{ {
  } };

#endif /* SOURCES_PMD_PHASECURRENTSENSOR_CONFIG_DESCRIPTION_H_ */
#endif /* SOURCES_PMD_PHASECURRENTSENSOR_CONFIG_DESCRIPTION_H_ */
