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

#include "CRC.h"
#include "trace.h"
#include "LockGuard.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR |
                                                        ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Crc;
using hal::Factory;

void Crc::initialize(void) const
{
    CRC_ResetDR();
}

uint32_t Crc::getCrc(uint8_t const* const data, const size_t bytes) const
{
    return getCrcUbiquitous<uint8_t>(data, bytes);
}

uint32_t Crc::getCrc(uint32_t const* const data, const size_t words) const
{
    return getCrcUbiquitous<uint32_t>(data, words);
}

template<typename T>
uint32_t Crc::getCrcUbiquitous(T const* const data, const size_t length) const
{
    static_assert(std::is_same<uint32_t, T>::value || std::is_same<uint8_t, T>::value, "Method not implemented!");

    if ((data == nullptr) || (length == 0)) {
        Trace(ZONE_WARNING, "Invalid parameters\r\n");
        return 0;
    }

    os::LockGuard<os::Mutex> lock(CrcUnitAvailableMutex[static_cast<size_t>(mDescription)]);

    CRC_ResetDR();
    for (size_t i = 0; i < length; i++) {
        CRC_CalcCRC(data[i]);
    }

    return CRC_GetCRC();
}

std::array<os::Mutex, Crc::Description::__ENUM__SIZE> Crc::CrcUnitAvailableMutex;
constexpr std::array<const Crc, Crc::__ENUM__SIZE> Factory<Crc>::Container;
