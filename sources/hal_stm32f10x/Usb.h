// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_USART_H_
#define SOURCES_PMD_USART_H_

#include <cstdint>
#include <chrono>
#include <string_view>

#include "hal_Factory.h"

namespace hal
{
struct Usb {
    Usb(const Usb&) = delete;
    Usb(Usb&&) = default;
    Usb& operator=(const Usb&) = delete;
    Usb& operator=(Usb&&) = delete;

    size_t send(const std::string_view&) const;

    size_t send(uint8_t const* const            data,
                const size_t                    length,
                const std::chrono::milliseconds timeout = std::chrono::milliseconds(500)) const;
    bool isReadyToSend(void) const;

    size_t receive(uint8_t* const, const size_t,
                   const std::chrono::milliseconds timeout = std::chrono::milliseconds(1)) const;
    size_t receiveAvailableData(uint8_t* const data, const size_t length) const;
    bool isReadyToReceive(void) const;

    bool isConfigured(void) const;
    void resetRxBuffer(void) const;

private:
    constexpr Usb(void){}

    void initialize(void) const;

    friend class Factory<Usb>;
};

template<>
class Factory<Usb>
{
    static constexpr const Usb mUsb{};

    Factory(void)
    {
        mUsb.initialize();
    }

public:
    static constexpr const Usb& get(void)
    {
        return mUsb;
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_USART_H_ */
