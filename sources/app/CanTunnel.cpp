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

#include "CanTunnel.h"
#include "trace.h"

using app::CanTunnel;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

CanTunnel::CanTunnel(const hal::UsartWithDma& tunnelInterface,
                     app::CanController&      canInterface) :
    os::DeepSleepModule(),
    mTunnelTask("TunnelTask",
                CanTunnel::STACKSIZE,
                os::Task::Priority::HIGH,
                [this](const bool& join)
                {
                    tunnelTaskFunction(join);
                }),
    mTunnelInterface(tunnelInterface),
    mCanInterface(canInterface)
{
    mCanInterface.registerReceiveCallback([&](std::string_view data){mTunnelInterface.send(data, 100);
                                          });
    mTunnelInterface.mUsart.enableNonBlockingReceive(
                                                     [&](uint8_t data)
                                                     {const char* d = reinterpret_cast<const char*>(&data);
                                                      mCanInterface.send(std::string_view(d, 1), 100);
                                                     });
}

void CanTunnel::enterDeepSleep(void)
{
    mTunnelTask.join();
}

void CanTunnel::exitDeepSleep(void)
{
    mTunnelTask.start();
}

void CanTunnel::tunnelTaskFunction(const bool& join)
{
    os::ThisTask::sleep(std::chrono::milliseconds(500));
    mCanInterface.on();
    mCanInterface.triggerFirmwareUpdate();
    os::ThisTask::sleep(std::chrono::milliseconds(100));
    while (mCanInterface.isPerformingFirmwareUpdate()) {
        os::ThisTask::sleep(std::chrono::milliseconds(200));
        mTunnelInterface.send(std::string("."), 100);
    }

    if (mCanInterface.wasFirmwareUpdateSuccessful() == false) {
        mTunnelInterface.send(std::string("\r\nERROR\r\n"), 100);
    } else {
        mCanInterface.on();
    }
    do {
        os::ThisTask::sleep(std::chrono::milliseconds(500));
    } while (!join);

    mCanInterface.off();
}
