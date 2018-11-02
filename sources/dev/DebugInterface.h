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

#ifndef SOURCES_PMD_DEBUGINTERFACE_H_
#define SOURCES_PMD_DEBUGINTERFACE_H_

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

#endif /* SOURCES_PMD_DEBUGINTERFACE_H_ */
