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

#include "pmd_CRC.h"
#include "trace.h"
#include "pmd_LockGuard.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR |
                                                        ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Crc;
using hal::Factory;

void Crc::initialize(void) const
{
    CRC_DeInit();
    CRC_ReverseInputDataSelect(mReverseInputSelection);
    CRC_ReverseOutputDataCmd(mReverseOutputSelection);
    CRC_SetInitRegister(mInitialValue);
    CRC_PolynomialSizeSelect(mPolynomialSize);
    CRC_SetPolynomial(mPolynomial);
}

uint8_t Crc::getCrc(uint8_t const* const data, const size_t length) const
{
    if ((data == nullptr) || (length == 0)) {
        Trace(ZONE_WARNING, "Invalid parameters\r\n");
        return 0;
    }

    os::LockGuard<os::Mutex> lock(CrcUnitAvailableMutex[static_cast<size_t>(mDescription)]);

    CRC_ResetDR();
    for (size_t i = 0; i < length; i++) {
        CRC_CalcCRC8bits(data[i]);
    }
    return CRC_GetCRC();
}

std::array<os::Mutex, Crc::Description::__ENUM__SIZE> Crc::CrcUnitAvailableMutex;
constexpr std::array<const Crc, Crc::__ENUM__SIZE> Factory<Crc>::Container;
