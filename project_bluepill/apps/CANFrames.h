// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2019 Nils Weiss
 */

#pragma once

#include "Can.h"
#include <cstddef>

namespace app
{
struct msg :
    CanTxMsg {
    enum class type : uint32_t {
        tempomat = 0x143,
        ignition = 0x23,
        start = 0x453,
        speed = 0x3c9,
        fuelLevel = 0x210,
        oilLevel = 0x116,
        engineTemperature = 0x156,
        engineRPM = 0x3ff,
        malfunction = 0x4ac
    };

    void setType(const msg::type t);
    msg::type getType(void) const;
    void setData(const uint32_t data);
    uint32_t getData(void) const;
};
}
