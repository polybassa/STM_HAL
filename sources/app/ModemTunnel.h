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

#ifndef SOURCES_PMD_MODEMTUNNEL_H_
#define SOURCES_PMD_MODEMTUNNEL_H_

#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "os_StreamBuffer.h"
#include "UsartWithDma.h"
#include "Gpio.h"

namespace app
{
class ModemTunnel final :
    private os::DeepSleepModule
{
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr size_t STACKSIZE = 1024;
    static constexpr size_t BUFFERSIZE = 128;

    static os::StreamBuffer<uint8_t, BUFFERSIZE> ModemReceiveBuffer;
    static os::StreamBuffer<uint8_t, BUFFERSIZE> TunnelReceiveBuffer;

    os::TaskInterruptable mModemTxTask;
    os::TaskInterruptable mTunnelTxTask;

    const hal::UsartWithDma& mTunnelInterface;
    const hal::UsartWithDma& mModemInterface;
    const hal::Gpio& mModemReset;
    const hal::Gpio& mModemPower;
    const hal::Gpio& mModemSupplyVoltage;

    void modemTxTaskFunction(const bool&);
    void tunnelTxTaskFunction(const bool&);

    void modemOn(void) const;
    void modemOff(void) const;
    void modemReset(void);

public:
    ModemTunnel(const hal::UsartWithDma & tunnelInterface,
                const hal::UsartWithDma & modemInterface,
                const hal::Gpio & resetPin,
                const hal::Gpio & powerPin,
                const hal::Gpio & supplyPin);

    ModemTunnel(const ModemTunnel &) = delete;
    ModemTunnel(ModemTunnel &&) = delete;
    ModemTunnel& operator=(const ModemTunnel&) = delete;
    ModemTunnel& operator=(ModemTunnel &&) = delete;

    static void TunnelInterruptHandler(uint8_t);
    static void ModemInterruptHandler(uint8_t);
};
}

#endif /* SOURCES_PMD_MODEMTUNNEL_H_ */