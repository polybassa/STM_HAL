// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include <cstdint>
#include <array>
#include "interface_Light.h"
#include "dev_Factory.h"
#include "SpiWithDma.h"

namespace dev
{
struct Light {
    Light() = delete;
    Light(const Light&) = delete;
    Light(Light&&) = default;
    Light& operator=(const Light&) = delete;
    Light& operator=(Light&&) = delete;

    void setColor(const interface ::Color& color) const;
    void displayNumber(const uint8_t number, const interface ::Color& color) const;

private:
    constexpr Light(const enum interface::Light::Description& desc, const hal::SpiWithDma& spi) :
        mDescription(desc), mSpi(spi) {}

    uint8_t getBitmask(const uint8_t) const;
    void convertByteToBitArray(const uint8_t byte, uint8_t* bitArray) const;

    const enum interface::Light::Description mDescription;
    const hal::SpiWithDma& mSpi;

    static const size_t LED_COUNT = 13;
    static const size_t COLORS_PER_LED = 3;
    static const size_t BYTE_TO_8BIT_SCALE_FACTOR = 4;

    static const size_t ARRAY_SIZE = LED_COUNT * BYTE_TO_8BIT_SCALE_FACTOR * COLORS_PER_LED;
    static std::array<std::array<uint8_t, ARRAY_SIZE>, interface ::Light::__ENUM__SIZE> LedBitArrays;

    friend class Factory<Light>;
};

template<>
class Factory<Light>
{
    static constexpr const std::array<const Light,
                                      interface ::Light::__ENUM__SIZE> Container =
    { {
          Light(interface ::Light::HEADLIGHT, hal::Factory<hal::SpiWithDma>::get<hal::Spi::HEADLIGHT>())
      } };

public:
    template<enum interface::Light::Description index>
    static constexpr const Light& get(void)
    {
        static_assert(index != interface ::Light::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}
