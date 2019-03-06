// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "trace.h"
#include "CANFrames.h"
#include <cstring>

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using app::msg;

void msg::setType(const msg::type t)
{
    StdId = static_cast<uint32_t>(t);
    IDE = 0;
    RTR = 0;
}

msg::type msg::getType(void) const
{
    return static_cast<msg::type>(StdId);
}

void msg::setData(const uint32_t data)
{
    memset(&Data, 0, sizeof(Data));

    switch (static_cast<msg::type>(StdId)) {
    case type::tempomat:
        Data[0] = 0xff000000 & data >> 24;
        Data[1] = 0xff0000 & data >> 16;
        Data[2] = 0xff00 & data >> 8;
        Data[3] = 0xff & data >> 0;
        DLC = 4;
        break;

    case type::ignition:
        {
            const auto ignitionData = data & 0x01;
            Data[4] = 0xff & ignitionData;
            Data[5] = ~(0xff & ignitionData);
            DLC = 6;
            break;
        }

    case type::start:
        {
            const auto startDate = data & 0x01;
            Data[0] = 0xff & startDate;
            Data[1] = ~(0xff & startDate);
            DLC = 2;
            break;
        }

    case type::speed:
        Data[0] = 0x00;
        Data[1] = 0x00;
        Data[2] = 0x00;
        Data[3] = 0x00;
        Data[4] = 0xff000000 & data >> 24;
        Data[5] = 0xff0000 & data >> 16;
        Data[6] = 0xff00 & data >> 8;
        Data[7] = 0xff & data >> 0;
        DLC = 8;
        break;

    case type::fuelLevel:
        // max 255 l
        Data[0] = 0xff & data;
        break;

    case type::oilLevel:
        // max. 15l
        Data[0] = 0x0f & data;
        break;

    case type::engineTemperature:
        // max 255
        Data[0] = 0x00;
        Data[1] = 0xff & data;
        break;

    case type::engineRPM:
        // max 12.287 (0x2FFF)
        Data[0] = 0x00;
        Data[1] = 0x00;
        Data[2] = 0x00;
        Data[3] = 0x2f00 & data >> 8;
        Data[4] = 0xff & data >> 0;
        DLC = 5;
        break;

    case type::malfunction:
        {
            const auto malData = data & 0x01;
            Data[3] = 0xff & malData;
            Data[4] = ~(0xff & malData);
            DLC = 5;
            break;
        }
    }
}

uint32_t msg::getData(void) const
{
    switch (static_cast<msg::type>(StdId)) {
    case type::tempomat:
        return (Data[0] << 24) +
               (Data[1] << 16) +
               (Data[2] << 8) +
               (Data[3] << 0);

    case type::ignition:
        return 0xff == (Data[4] ^ Data[5]) ? Data[4] : 0;

    case type::start:
        return 0xff == (Data[0] ^ Data[1]) ? Data[0] : 0;

    case type::speed:
        return (Data[4] << 24) +
               (Data[5] << 16) +
               (Data[6] << 8) +
               (Data[7] << 0);

    case type::fuelLevel:
        return Data[0];

    case type::oilLevel:
        return Data[0];

    case type::engineTemperature:
        return Data[1];

    case type::engineRPM:
        return (Data[3] << 8) +
               (Data[4] << 0);

    case type::malfunction:
        return 0xff == (Data[3] ^ Data[4]) ? Data[3] : 0;
    }
}
