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

#ifndef SOURCES_PMD_LIGHT_H_
#define SOURCES_PMD_LIGHT_H_

#include <cstdint>
#include <array>
#include "interface_Light.h"
#include "dev_Factory.h"
#include "SpiWithDma.h"

namespace dev
{
struct Light :
    interface::Light<Light> {
    Light() = delete;
    Light(const Light&) = delete;
    Light(Light &&) = default;
    Light& operator=(const Light&) = delete;
    Light& operator=(Light &&) = delete;

    void setColor(const interface::Color& color) const;
    void displayNumber(const uint8_t number, const interface::Color& color) const;

private:
    constexpr Light(const enum Description& desc, const hal::SpiWithDma& spi) :
        mDescription(desc), mSpi(spi) {}

    uint8_t getBitmask(const uint8_t) const;
    void convertByteToBitArray(const uint8_t byte, uint8_t* bitArray) const;

    const enum Description mDescription;
    const hal::SpiWithDma& mSpi;

    static const size_t LED_COUNT = 13;
    static const size_t COLORS_PER_LED = 3;
    static const size_t BYTE_TO_8BIT_SCALE_FACTOR = 4;

    static const size_t ARRAY_SIZE = LED_COUNT * BYTE_TO_8BIT_SCALE_FACTOR * COLORS_PER_LED;
    static std::array<std::array<uint8_t, ARRAY_SIZE>, __ENUM__SIZE> LedBitArrays;

    friend class Factory<Light>;
};

template<>
class Factory<Light>
{
    static constexpr const std::array<const Light,
                                      Light::__ENUM__SIZE> Container =
    { {
          Light(Light::HEADLIGHT, hal::Factory<hal::SpiWithDma>::get<hal::Spi::HEADLIGHT>()),
          Light(Light::BACKLIGHT, hal::Factory<hal::SpiWithDma>::get<hal::Spi::BACKLIGHT>())
      } };

public:
    template<enum Light::Description index>
    static constexpr const Light& get(void)
    {
        static_assert(index != Light::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_LIGHT_H_ */
