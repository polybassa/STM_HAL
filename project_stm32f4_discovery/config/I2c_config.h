// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_I2C_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_I2C_CONFIG_DESCRIPTION_H_

enum Description {
    __ENUM__SIZE
};
#else
#ifndef SOURCES_PMD_I2C_CONFIG_CONTAINER_H_
#define SOURCES_PMD_I2C_CONFIG_CONTAINER_H_

/* ATTENTION: Don't forget do add all necessary
 * clock domains to the Factory<I2c>::Clocks array */

static constexpr const std::array<const I2c, I2c::__ENUM__SIZE> Container =
{ {
  }};

static constexpr const std::array<const uint32_t, I2c::__ENUM__SIZE> Clocks =
{ {
  }};

#endif /* SOURCES_PMD_I2C_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_I2C_CONFIG_DESCRIPTION_H_ */
