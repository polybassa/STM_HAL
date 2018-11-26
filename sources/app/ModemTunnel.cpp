// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "ModemTunnel.h"
#include "trace.h"

using app::ModemTunnel;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

os::StreamBuffer<uint8_t, ModemTunnel::BUFFERSIZE> ModemTunnel::ModemReceiveBuffer;
os::StreamBuffer<uint8_t, ModemTunnel::BUFFERSIZE> ModemTunnel::TunnelReceiveBuffer;

void ModemTunnel::TunnelInterruptHandler(uint8_t data)
{
    TunnelReceiveBuffer.sendFromISR(data);
}

void ModemTunnel::ModemInterruptHandler(uint8_t data)
{
    ModemReceiveBuffer.sendFromISR(data);
}

ModemTunnel::ModemTunnel(const hal::UsartWithDma& tunnelInterface,
                         const hal::UsartWithDma& modemInterface,
                         const hal::Gpio&         resetPin,
                         const hal::Gpio&         powerPin,
                         const hal::Gpio&         supplyPin) :
    os::DeepSleepModule(),
    mModemTxTask("ModemTxTask",
                 ModemTunnel::STACKSIZE,
                 os::Task::Priority::HIGH,
                 [this](const bool& join)
{
    modemTxTaskFunction(join);
}),
    mTunnelTxTask("TunnelTxTask",
                  ModemTunnel::STACKSIZE,
                  os::Task::Priority::HIGH,
                  [this](const bool& join)
{
    tunnelTxTaskFunction(join);
}),
    mTunnelInterface(tunnelInterface),
    mModemInterface(modemInterface),
    mModemReset(resetPin),
    mModemPower(powerPin),
    mModemSupplyVoltage(supplyPin)
{
    mModemInterface.mUsart.enableNonBlockingReceive(ModemInterruptHandler);
    mTunnelInterface.mUsart.enableNonBlockingReceive(TunnelInterruptHandler);
}

void ModemTunnel::enterDeepSleep(void)
{
    modemOff();
    mModemTxTask.join();
    mTunnelTxTask.join();
}

void ModemTunnel::exitDeepSleep(void)
{
    mModemTxTask.start();
    mTunnelTxTask.start();
}

void ModemTunnel::modemTxTaskFunction(const bool& join)
{
    os::ThisTask::sleep(std::chrono::milliseconds(500));
    modemReset();

    do {
        uint8_t byte = 0;
        TunnelReceiveBuffer.receive(byte);
        mModemInterface.send(&byte, 1);
    } while (!join);
}

void ModemTunnel::tunnelTxTaskFunction(const bool& join)
{
    os::ThisTask::sleep(std::chrono::milliseconds(500));
    do {
        uint8_t byte = 0;
        ModemReceiveBuffer.receive(byte);
        mTunnelInterface.send(&byte, 1);
    } while (!join);
}

void ModemTunnel::modemOn(void) const
{
    mModemSupplyVoltage = true;
}

void ModemTunnel::modemOff(void) const
{
    mModemReset = false;
    mModemPower = false;
    mModemSupplyVoltage = false;
}

void ModemTunnel::modemReset(void)
{
    modemOff();
    os::ThisTask::sleep(std::chrono::milliseconds(1000));
    modemOn();
    os::ThisTask::sleep(std::chrono::milliseconds(2000));
}
