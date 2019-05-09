// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_CRC_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_CRC_CONFIG_DESCRIPTION_H_

enum Description {
    SYSTEM_CRC,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_CRC_CONFIG_CONTAINER_H_
#define SOURCES_PMD_CRC_CONFIG_CONTAINER_H_

static constexpr std::array<const Crc, Crc::__ENUM__SIZE> Container =
{ {
      Crc(Crc::SYSTEM_CRC)    // on the stm32f4 the crc module is not configurable
  } };

#endif /* SOURCES_PMD_CRC_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_CRC_CONFIG_DESCRIPTION_H_ */
