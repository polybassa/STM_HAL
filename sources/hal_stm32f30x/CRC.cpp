// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "CRC.h"
#include "trace.h"
#include "LockGuard.h"

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
