// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "CanTunnel.h"
#include "trace.h"

using app::CanTunnel;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

os::StreamBuffer<uint8_t, CanTunnel::BUFFERSIZE> CanTunnel::CanReceiveBuffer;
os::StreamBuffer<uint8_t, CanTunnel::BUFFERSIZE> CanTunnel::TunnelReceiveBuffer;

CanTunnel::CanTunnel(const hal::UsartWithDma& tunnelInterface,
                     app::CanController&      canInterface) :
    os::DeepSleepModule(),
    mTunnelTxTask("TunnelTask",
                  CanTunnel::STACKSIZE,
                  os::Task::Priority::HIGH,
                  [this](const bool& join)
{
    tunnelTxTaskFunction(join);
}),
    mCanTxTask("ModemTxTask",
               CanTunnel::STACKSIZE,
               os::Task::Priority::HIGH,
               [this](const bool& join)
{
    canTxTaskFunction(join);
}),
    mTunnelInterface(tunnelInterface),
    mCanInterface(canInterface)
{}

void CanTunnel::enterDeepSleep(void)
{
    mTunnelTxTask.join();
}

void CanTunnel::exitDeepSleep(void)
{
    mTunnelTxTask.start();
}

void CanTunnel::tunnelTxTaskFunction(const bool& join)
{
    uint8_t doFlashing = 0;
    mTunnelInterface.send(std::string("\r\nPress 1 for flashing seco, anything else to boot!\r\n"), 100);
    mTunnelInterface.mUsart.receive(&doFlashing, 1);

    if (doFlashing == '1') {
        mTunnelInterface.send(std::string("\r\nFLASHING SECO\r\n"), 100);
        os::ThisTask::sleep(std::chrono::milliseconds(500));
        mCanInterface.on();
        mCanInterface.triggerFirmwareUpdate();
        os::ThisTask::sleep(std::chrono::milliseconds(100));
        while (mCanInterface.isPerformingFirmwareUpdate()) {
            os::ThisTask::sleep(std::chrono::milliseconds(200));
            mTunnelInterface.send(std::string("."), 100);
        }
    }

    if ((doFlashing == '1') && (mCanInterface.wasFirmwareUpdateSuccessful() == false)) {
        mTunnelInterface.send(std::string("\r\nERROR\r\n"), 100);
    } else {
        mCanInterface.off();
        mCanInterface.mInterface.mUsart.enableNonBlockingReceive([&](uint8_t data)
        {
            CanReceiveBuffer.sendFromISR(data);
        });

        mTunnelInterface.mUsart.enableNonBlockingReceive([&](uint8_t data)
        {
            TunnelReceiveBuffer.sendFromISR(data);
        });

        os::ThisTask::sleep(std::chrono::milliseconds(200));
        mCanInterface.on();
    }
    do {
        uint8_t byte = 0;
        CanReceiveBuffer.receive(byte);
        mTunnelInterface.mUsart.send(&byte, 1);
    } while (!join);

    mCanInterface.off();
}

void CanTunnel::canTxTaskFunction(const bool& join)
{
    do {
        uint8_t byte = 0;
        TunnelReceiveBuffer.receive(byte);
        mCanInterface.mInterface.mUsart.send(&byte, 1);
    } while (!join);

    mCanInterface.off();
}
