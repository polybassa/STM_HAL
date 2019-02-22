// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "TaskInterruptable.h"
#include "os_StreamBuffer.h"
#include "UsartWithDma.h"
#include "Gpio.h"
#include <string_view>
#include <array>

namespace app
{
class CanTunnel;

class CanController final
{
    static constexpr size_t STACKSIZE = 1024;
    static constexpr size_t BUFFERSIZE = 2048;
    static constexpr size_t MAXCHUNKSIZE = 512;

    static os::StreamBuffer<char, BUFFERSIZE> ReceiveBuffer;
    std::array<char, MAXCHUNKSIZE> mTempReceiveCallbackBuffer;

    os::TaskInterruptable mTask;

    const hal::UsartWithDma& mInterface;
    const hal::Gpio& mCanSupplyVoltage;
    const hal::Gpio& mUsartTxPin;

    std::function<void(std::string_view)> mReceiveCallback;

    bool mIsPerformingFirmwareUpdate = false;
    bool mWasFirmwareUpdateSuccessful = false;

    void taskFunction(const bool&);
    void flashSecCoFirmware(void);
    bool flash(std::string_view data, size_t length);
    void resetToBootloader(void);
    bool connectToBootloader(void);
    bool sendCommandToBootloader(std::string_view cmd);
    uint32_t swap(const uint32_t);
    bool receiveResponseFromBootloader(void);
    size_t sendToBootloaderWithChecksum(std::string_view, uint8_t initvalue = 0);
    bool eraseChip(void);
    bool sendGoCommand(const uint32_t goAddress);

public:
    CanController(const hal::UsartWithDma& interface,
                  const hal::Gpio&         supplyPin,
                  const hal::Gpio&         usartTxPin);

    CanController(const CanController&) = delete;
    CanController(CanController&&) = delete;
    CanController& operator=(const CanController&) = delete;
    CanController& operator=(CanController&&) = delete;
    ~CanController(void);

    static void CanControllerInterruptHandler(uint8_t);

    void on(void);
    void off(void);
    void triggerFirmwareUpdate(void);
    bool isPerformingFirmwareUpdate(void) const;
    bool wasFirmwareUpdateSuccessful(void) const;

    size_t send(std::string_view, const uint32_t ticksToWait = portMAX_DELAY);
    size_t receive(uint8_t*, size_t, uint32_t ticksToWait = portMAX_DELAY);
    size_t bytesAvailable(void) const;

    void registerReceiveCallback(std::function<void(std::string_view)> );
    void unregisterReceiveCallback(void);

    friend CanTunnel;
};
}
