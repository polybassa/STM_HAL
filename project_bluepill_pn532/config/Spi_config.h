// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_SPI_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_SPI_CONFIG_DESCRIPTION_H_

enum Description {
    PN532SPI,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_SPI_CONFIG_CONTAINER_H_
#define SOURCES_PMD_SPI_CONFIG_CONTAINER_H_

static constexpr const std::array<const Spi, Spi::__ENUM__SIZE> Container =
{ {
      Spi(Spi::PN532SPI,
          SPI1_BASE,
          SPI_InitTypeDef { SPI_Direction_2Lines_FullDuplex, SPI_Mode_Master, SPI_DataSize_8b, SPI_CPOL_Low,
                            SPI_CPHA_1Edge, SPI_NSS_Soft, SPI_BaudRatePrescaler_64, SPI_FirstBit_LSB, 1 })
  } };

static constexpr const std::array<const uint32_t, Spi::__ENUM__SIZE> Clocks =
{ {
      RCC_APB2Periph_SPI1,
  } };

#endif /* SOURCES_PMD_SPI_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_SPI_CONFIG_DESCRIPTION_H_ */
