// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_DAC_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_DAC_CONFIG_DESCRIPTION_H_

enum Description {
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_DAC_CONFIG_CONTAINER_H_
#define SOURCES_PMD_DAC_CONFIG_CONTAINER_H_

static constexpr std::array<const Dac, Dac::Description::__ENUM__SIZE> Container =
{ {
      Dac(Dac::Description::__ENUM_SIZE, 0, 0, {0, 0, 0, 0}, 0)
  } };

#endif /* SOURCES_PMD_DAC_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_DAC_CONFIG_DESCRIPTION_H_ */
