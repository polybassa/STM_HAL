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
        TunnelReceiveBuffer.receive(byte, portMAX_DELAY);
        mModemInterface.send(&byte, 1);
    } while (!join);
}

void ModemTunnel::tunnelTxTaskFunction(const bool& join)
{
    os::ThisTask::sleep(std::chrono::milliseconds(500));
    do {
        uint8_t byte = 0;
        ModemReceiveBuffer.receive(byte, portMAX_DELAY);
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
