// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_SPIWITHDMA_CONFIG_CONTAINER_H_
#define SOURCES_PMD_SPIWITHDMA_CONFIG_CONTAINER_H_

static constexpr const std::array<const SpiWithDma, Spi::__ENUM__SIZE> Container =
{ {
      SpiWithDma(&Factory<Spi>::get<Spi::HEADLIGHT>(), SPI_I2S_DMAReq_Tx, &Factory<Dma>::get<Dma::SPI3_TX>())
  } };
#endif /* SOURCES_PMD_SPI_CONFIG_CONTAINER_H_ */
