// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_CRC_H_
#define SOURCES_PMD_CRC_H_

#include <cstdint>
#include <array>
#include "stm32f30x_crc.h"
#include "stm32f30x_rcc.h"
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

    uint8_t getCrc(uint8_t const* const data, const size_t length) const;

private:
    constexpr Crc(const enum Description desc,
                  const uint32_t         polynomialSize,
                  const uint32_t         reverseInputSelection,
                  const bool             reverseOutputSelection,
                  const uint32_t         initialValue,
                  const uint32_t         polynomial) :
        mDescription(std::move(desc)),
        mPolynomialSize(std::move(polynomialSize)),
        mReverseInputSelection(std::move(reverseInputSelection)),
        mReverseOutputSelection(reverseOutputSelection ==
                                true ? ENABLE : DISABLE),
        mInitialValue(std::move(initialValue)),
        mPolynomial(std::move(polynomial)) {}

    const enum Description mDescription;
    const uint32_t mPolynomialSize;
    const uint32_t mReverseInputSelection;
    const FunctionalState mReverseOutputSelection;
    const uint32_t mInitialValue;
    const uint32_t mPolynomial;

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
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);

        CRC_DeInit();

        Container[Crc::SYSTEM_CRC].initialize();
    }

public:

    template<enum Crc::Description index>
    static constexpr const Crc& get(void)
    {
        static_assert(IS_CRC_REVERSE_INPUT_DATA(Container[index].mReverseInputSelection), "Invalid");
        static_assert(IS_CRC_POL_SIZE(Container[index].mPolynomialSize), "Invalid");

        static_assert(index != Crc::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_CRC_H_ */
