// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "os_StreamBuffer.h"
#include "UsartWithDma.h"
#include "CanController.h"

namespace app
{
class CanTunnel final :
    private os::DeepSleepModule
{
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr size_t STACKSIZE = 1024;
    static constexpr size_t BUFFERSIZE = 128;

    static os::StreamBuffer<uint8_t, BUFFERSIZE> CanReceiveBuffer;
    static os::StreamBuffer<uint8_t, BUFFERSIZE> TunnelReceiveBuffer;

    os::TaskInterruptable mTunnelTxTask;
    os::TaskInterruptable mCanTxTask;

    const hal::UsartWithDma& mTunnelInterface;
    app::CanController& mCanInterface;

    void tunnelTxTaskFunction(const bool&);
    void canTxTaskFunction(const bool&);

public:
    CanTunnel(const hal::UsartWithDma& tunnelInterface,
              app::CanController&      canInterface);

    CanTunnel(const CanTunnel&) = delete;
    CanTunnel(CanTunnel&&) = delete;
    CanTunnel& operator=(const CanTunnel&) = delete;
    CanTunnel& operator=(CanTunnel&&) = delete;
};
}
