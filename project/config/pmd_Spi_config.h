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

#ifndef SOURCES_PMD_SPI_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_SPI_CONFIG_DESCRIPTION_H_

enum Description {
    BACKLIGHT, HEADLIGHT, NWP_SPI, __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_SPI_CONFIG_CONTAINER_H_
#define SOURCES_PMD_SPI_CONFIG_CONTAINER_H_

static constexpr const std::array<const Spi, Spi::__ENUM__SIZE> Container =
{ {
      Spi(Spi::BACKLIGHT,
          SPI2_BASE,
          SPI_InitTypeDef { SPI_Direction_1Line_Tx, SPI_Mode_Master, SPI_DataSize_8b, SPI_CPOL_High,
                            SPI_CPHA_2Edge, SPI_NSS_Soft, SPI_BaudRatePrescaler_16, SPI_FirstBit_MSB, 1 }),
      Spi(Spi::HEADLIGHT,
          SPI3_BASE,
          SPI_InitTypeDef { SPI_Direction_1Line_Tx, SPI_Mode_Master, SPI_DataSize_8b, SPI_CPOL_High,
                            SPI_CPHA_2Edge, SPI_NSS_Soft, SPI_BaudRatePrescaler_16, SPI_FirstBit_MSB, 1 }),
      Spi(Spi::NWP_SPI,
          SPI4_BASE,
          SPI_InitTypeDef { SPI_Direction_2Lines_FullDuplex, SPI_Mode_Master, SPI_DataSize_8b, SPI_CPOL_Low,
                            SPI_CPHA_1Edge, SPI_NSS_Hard, SPI_BaudRatePrescaler_4, SPI_FirstBit_MSB, 1 })
  } };

static constexpr const std::array<const uint32_t, Spi::__ENUM__SIZE> Clocks =
{ {
      RCC_APB1Periph_SPI2,
      RCC_APB1Periph_SPI3,
      RCC_APB2Periph_SPI4
  } };

#endif /* SOURCES_PMD_SPI_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_SPI_CONFIG_DESCRIPTION_H_ */
