// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_USART_H_
#define SOURCES_PMD_USART_H_

#include <cstdint>
#include <limits>
#include <array>
#include <functional>
#include "stm32f30x_usart.h"
#include "stm32f30x_rcc.h"
#include "hal_Factory.h"

extern "C" {
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);
void UART4_IRQHandler(void);
void UART5_IRQHandler(void);
}

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

    void enableReceiveTimeout(std::function<void(void)> callback, const size_t) const;
    void enableNonBlockingReceive(std::function<void(uint8_t)> callback) const;

    void disableReceiveTimeout(void) const;
    void disableNonBlockingReceive(void) const;

    void enableReceiveTimeoutIT_Flag(void) const;
    void disableReceiveTimeoutIT_Flag(void) const;

    static void USART_IRQHandler(const Usart& peripherie);

private:
    constexpr Usart(const enum Description&  desc,
                    const uint32_t&          peripherie,
                    const USART_InitTypeDef& conf,
                    const bool               txPinActiveLevelInversion = false) :
        mDescription(desc), mPeripherie(peripherie),
        mConfiguration(conf), mTxPinActiveLevelInversion(txPinActiveLevelInversion) {}

    const uint32_t mPeripherie;
    const USART_InitTypeDef mConfiguration;
    const bool mTxPinActiveLevelInversion;
    mutable bool mInitalized = false;

    void initialize(void) const;
    IRQn getIRQn(void) const;

    using ReceiveCallbackArray = std::array<std::function<void (uint8_t)>, Usart::__ENUM__SIZE>;
    using ReceiveTimeoutCallbackArray = std::array<std::function<void (void)>, Usart::__ENUM__SIZE>;

    static ReceiveTimeoutCallbackArray ReceiveTimeoutInterruptCallbacks;
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
        // TODO support all clocks
        for (const auto& clock : Clocks) {
            if (clock == RCC_APB2Periph_USART1) {
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
        static_assert(index < Container[index + 1].mDescription, "Incorrect order of instances in Factory");
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
