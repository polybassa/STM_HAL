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

#ifndef SOURCES_PMD_SPIWITHDMA_CONFIG_CONTAINER_H_
#define SOURCES_PMD_SPIWITHDMA_CONFIG_CONTAINER_H_

static constexpr const std::array<const SpiWithDma, Spi::__ENUM__SIZE> Container =
{ {
      SpiWithDma(&Factory<Spi>::get<Spi::BACKLIGHT>(), SPI_I2S_DMAReq_Tx, &Factory<Dma>::get<Dma::SPI2_TX>()),
      SpiWithDma(&Factory<Spi>::get<Spi::HEADLIGHT>(), SPI_I2S_DMAReq_Tx, &Factory<Dma>::get<Dma::SPI3_TX>())
  } };
#endif /* SOURCES_PMD_SPI_CONFIG_CONTAINER_H_ */
