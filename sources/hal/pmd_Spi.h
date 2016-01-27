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

#ifndef SOURCES_PMD_SPI_H_
#define SOURCES_PMD_SPI_H_

#include <cstdint>
#include <array>
#include "stm32f30x_spi.h"
#include "stm32f30x_rcc.h"
#include "stm32f30x.h"
#include "pmd_hal_Factory.h"
#include "pmd_Mutex.h"

namespace hal
{
struct Spi {
#include "pmd_Spi_config.h"

    const enum Description mDescription;

    Spi() = delete;
    Spi(const Spi&) = delete;
    Spi(Spi&&) = default;
    Spi& operator=(const Spi&) = delete;
    Spi& operator=(Spi&&) = delete;

    template<size_t n>
    size_t receive(std::array<uint8_t, n>&) const;

    template<size_t n>
    size_t send(const std::array<uint8_t, n>&) const;

    void send(const uint8_t) const;
    size_t send(uint8_t const* const, const size_t) const;
    bool isReadyToSend(void) const;

    uint8_t receive(void) const;
    size_t receive(uint8_t* const, const size_t) const;
    bool isReadyToReceive(void) const;

private:
    constexpr Spi(const enum Description& desc,
                  const uint32_t&         peripherie,
                  const SPI_InitTypeDef&  conf) :
        mDescription(desc), mPeripherie(peripherie), mConfiguration(conf) {}

    const uint32_t mPeripherie;
    const SPI_InitTypeDef mConfiguration;

    void initialize(void) const;

    static std::array<os::Mutex, Description::__ENUM__SIZE> InterfaceAvailableMutex;

    friend class Factory<Spi>;
    friend struct SpiWithDma;
};

template<>
class Factory<Spi> {
#include "pmd_Spi_config.h"

    Factory(void)
    {
        // TODO: Support all clock domains
        for (const auto& clock : Clocks) {
            if ((clock == RCC_APB1Periph_SPI2) || (clock == RCC_APB1Periph_SPI3)) {
                RCC_APB1PeriphClockCmd(clock, ENABLE);
            }
            if ((clock == RCC_APB2Periph_SPI1) || (clock == RCC_APB2Periph_SPI4)) {
                RCC_APB2PeriphClockCmd(clock, ENABLE);
            }
        }

        for (const auto& Spi : Container) {
            Spi.initialize();
        }
    }

public:
    template<enum Spi::Description index>
    static constexpr const Spi& get(void)
    {
        static_assert(IS_SPI_ALL_PERIPH_BASE(Container[index].mPeripherie), "Invalid");
        static_assert(IS_SPI_BAUDRATE_PRESCALER(Container[index].mConfiguration.SPI_BaudRatePrescaler), "Invalid");
        static_assert(IS_SPI_CPHA(Container[index].mConfiguration.SPI_CPHA), "Invalid");
        static_assert(IS_SPI_CPOL(Container[index].mConfiguration.SPI_CPOL), "Invalid");
        static_assert(IS_SPI_CRC_POLYNOMIAL(Container[index].mConfiguration.SPI_CRCPolynomial), "Invalid");
        static_assert(IS_SPI_DATA_SIZE(Container[index].mConfiguration.SPI_DataSize), "Invalid");
        static_assert(IS_SPI_DIRECTION_MODE(Container[index].mConfiguration.SPI_Direction), "Invalid");
        static_assert(IS_SPI_FIRST_BIT(Container[index].mConfiguration.SPI_FirstBit), "Invalid");
        static_assert(IS_SPI_MODE(Container[index].mConfiguration.SPI_Mode), "Invalid");
        static_assert(IS_SPI_NSS(Container[index].mConfiguration.SPI_NSS), "Invalid");

        static_assert(index != Spi::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_SPI_H_ */
