/* Copyright (C) 2018  Nils Weiss and Henning Mende
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

#ifndef SOURCES_PMD_CRC_H_
#define SOURCES_PMD_CRC_H_

#include <cstdint>
#include <array>
#include "stm32f4xx_crc.h"
#include "stm32f4xx_rcc.h"
#include "Mutex.h"
#include "hal_Factory.h"

namespace hal
{
struct Crc {
#include "CRC_config.h"

    Crc() = delete;
    Crc(const Crc&) = delete;
    Crc(Crc&&) = default;
    Crc& operator=(const Crc&) = delete;
    Crc& operator=(Crc&&) = delete;

    uint32_t getCrc(uint8_t const* const data, const size_t bytes) const;
    uint32_t getCrc(uint32_t const* const data, const size_t words) const;

    template<typename T>
    uint32_t getCrc(T const* const data, const size_t length) const;

private:
    constexpr Crc(const enum Description desc) :
        mDescription(std::move(desc)) {}

    const enum Description mDescription;

    void initialize(void) const;

    static std::array<os::Mutex, Description::__ENUM__SIZE> CrcUnitAvailableMutex;

    friend class Factory<Crc>;
};

template<>
class Factory<Crc>
{
#include "CRC_config.h"

    Factory(void)
    {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);

//        CRC_DeInit(); // TODO probably not needed?

        Container[Crc::SYSTEM_CRC].initialize();
    }

public:

    template<enum Crc::Description index>
    static constexpr const Crc& get(void)
    {
        static_assert(index != Crc::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_CRC_H_ */
