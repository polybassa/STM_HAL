// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

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
    ModemTunnel(const hal::UsartWithDma& tunnelInterface,
                const hal::UsartWithDma& modemInterface,
                const hal::Gpio&         resetPin,
                const hal::Gpio&         powerPin,
                const hal::Gpio&         supplyPin);

    ModemTunnel(const ModemTunnel&) = delete;
    ModemTunnel(ModemTunnel&&) = delete;
    ModemTunnel& operator=(const ModemTunnel&) = delete;
    ModemTunnel& operator=(ModemTunnel&&) = delete;

    static void TunnelInterruptHandler(uint8_t);
    static void ModemInterruptHandler(uint8_t);
};
}
