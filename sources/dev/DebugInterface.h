// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "Usart.h"
#include "hal_Factory.h"
#include "Mutex.h"

namespace dev
{
class DebugInterface
{
    static constexpr auto& interface = hal::Factory<hal::Usart>::get<hal::Usart::DEBUG_IF>();

#if defined(DEBUG)
    static const size_t INTERNAL_BUFFER_SIZE = 1024;
    static os::Mutex PrintMutex;
    static std::array<uint8_t, INTERNAL_BUFFER_SIZE> printBuffer;
#endif

public:
    DebugInterface(void);
    DebugInterface(const DebugInterface&) = delete;
    DebugInterface(DebugInterface&&) = delete;
    DebugInterface& operator=(const DebugInterface&) = delete;
    DebugInterface& operator=(DebugInterface&&) = delete;
    virtual ~DebugInterface();

    void printStartupMessage(void) const;
    void clearTerminal(void) const;
    void printf(const char*, ...) const;
    void send(const char* str, const size_t len) const;
    bool dataAvailable(void) const;
    size_t receiveAvailableData(uint8_t* const data, const size_t length) const;
};
}
