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

#ifndef SOURCES_PMD_USART_H_
#define SOURCES_PMD_USART_H_

#include <cstdint>
#include <limits>
#include <array>
#include <functional>
#include "stm32f4xx_usart.h"
#include "stm32f4xx_rcc.h"
#include "hal_Factory.h"
#include "Nvic.h"

// ================================================================================================
// Added preprocessor macros that are not part of the stm32f4 std periph lib,
// but of those for the other stm32fx controllers.
#define IS_USART_ALL_PERIPH_BASE(PERIPH) (((PERIPH) == USART1_BASE) || \
                                          ((PERIPH) == USART2_BASE) || \
                                          ((PERIPH) == USART3_BASE) || \
                                          ((PERIPH) == UART4_BASE) || \
                                          ((PERIPH) == UART5_BASE) || \
                                          ((PERIPH) == USART6_BASE))
// ================================================================================================

namespace hal
{
struct Usart {
#include "Usart_config.h"

    const enum Description mDescription;

    Usart() = delete;
    Usart(const Usart&) = delete;
    Usart(Usart&&) = default;
    Usart& operator=(const Usart&) = delete;
    Usart& operator=(Usart&&) = delete;

    template<size_t n>
    size_t send(const std::array<uint8_t, n>&) const;

    void send(const uint16_t) const;
    size_t send(uint8_t const* const, const size_t) const;
    bool isReadyToSend(void) const;

    template<size_t n>
    size_t receive(std::array<uint8_t, n>&) const;

    uint16_t receive(void) const;
    size_t receive(uint8_t* const, const size_t) const;
    size_t receiveAvailableData(uint8_t* const data, const size_t length) const;
    bool isReadyToReceive(void) const;

    bool hasOverRunError(void) const;
    bool hasNoiseError(void) const;
    bool hasFramingError(void) const;
    bool hasParityError(void) const;

    void clearOverRunError(void) const;
    void clearNoiseError(void) const;
    void clearFramingError(void) const;
    void clearParityError(void) const;

    bool isInitalized(void) const;

    void setBaudRate(const size_t) const;

    void enableNonBlockingReceive(std::function<void(uint8_t)> callback) const;

    void disableNonBlockingReceive(void) const;

private:
    constexpr Usart(const enum Description&  desc,
                    const uint32_t&          peripherie,
                    const USART_InitTypeDef& conf,
                    const Nvic&              nvic) :
        mDescription(desc),
        mPeripherie(peripherie),
        mConfiguration(conf),
        mNvic(nvic) {}

    const uint32_t mPeripherie;
    const USART_InitTypeDef mConfiguration;
    mutable bool mInitalized = false;
    const Nvic& mNvic;

    void initialize(void) const;

    using ReceiveCallbackArray = std::array<std::function<void (uint8_t)>, Usart::__ENUM__SIZE>;

    static ReceiveCallbackArray ReceiveInterruptCallbacks;

    friend class Factory<Usart>;
    friend struct UsartWithDma;
};

template<size_t n>
size_t Usart::send(const std::array<uint8_t, n>& tx) const
{
    return send(tx.data(), tx.size());
}

template<size_t n>
size_t Usart::receive(std::array<uint8_t, n>& rx) const
{
    return receive(rx.data(), rx.size());
}

template<>
class Factory<Usart>
{
#include "Usart_config.h"

    Factory(void)
    {
        for (const auto& clock : Clocks) {
            if ((clock == RCC_APB2Periph_USART1) ||
                (clock == RCC_APB2Periph_USART6))
            {
                RCC_APB2PeriphClockCmd(clock, ENABLE);
            } else {
                RCC_APB1PeriphClockCmd(clock, ENABLE);
            }
        }
        for (const auto& usart : Container) {
            if (usart.mDescription != Usart::__ENUM__SIZE) {
                usart.initialize();
            }
        }
    }

    template<uint32_t peripherieBase, enum Usart::Description index>
    static constexpr const Usart& getByPeripherie(void)
    {
        return (Container[index]).mPeripherie ==
               peripherieBase ? Container[index] : getByPeripherie<peripherieBase,
                                                                   static_cast<enum Usart::Description>(index - 1)>();
    }

public:
    template<enum Usart::Description index>
    static constexpr const Usart& get(void)
    {
        static_assert(IS_USART_ALL_PERIPH_BASE(Container[index].mPeripherie), "Invalid Peripheries ");
        static_assert(IS_USART_WORD_LENGTH(Container[index].mConfiguration.USART_WordLength), "Invalid WordLength");
        static_assert(IS_USART_STOPBITS(Container[index].mConfiguration.USART_StopBits), "Invalid StopBits");
        static_assert(IS_USART_PARITY(Container[index].mConfiguration.USART_Parity), "Invalid Parity");
        static_assert(IS_USART_MODE(Container[index].mConfiguration.USART_Mode), "Invalid Mode ");
        static_assert(IS_USART_HARDWARE_FLOW_CONTROL(
                                                     Container[index].mConfiguration.USART_HardwareFlowControl),
                      "Invalid HwFlowCtrl");
        static_assert(index != Usart::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<uint32_t peripherieBase>
    static constexpr const Usart& getByPeripherie(void)
    {
        static_assert(IS_USART_ALL_PERIPH_BASE(peripherieBase), "Invalid Peripheries ");
        return getByPeripherie<peripherieBase,
                               static_cast<enum Usart::Description>(Usart::Description::__ENUM__SIZE - 1)>();
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_USART_H_ */
