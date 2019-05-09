// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_SPI_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_SPI_CONFIG_DESCRIPTION_H_

enum Description {
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_SPI_CONFIG_CONTAINER_H_
#define SOURCES_PMD_SPI_CONFIG_CONTAINER_H_

static constexpr const std::array<const Spi, Spi::__ENUM__SIZE> Container =
{ {
  } };

static constexpr const std::array<const uint32_t, Spi::__ENUM__SIZE> Clocks =
{ {
  } };

#endif /* SOURCES_PMD_SPI_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_SPI_CONFIG_DESCRIPTION_H_ */
