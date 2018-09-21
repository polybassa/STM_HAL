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

#ifndef SOURCES_PMD_CANCONTROLLER_H_
#define SOURCES_PMD_CANCONTROLLER_H_

#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "os_StreamBuffer.h"
#include "Semaphore.h"
#include "UsartWithDma.h"
#include "Gpio.h"
#include <string>
#include <string_view>
#include <array>

namespace app
{
class CanController final :
    private os::DeepSleepModule
{
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr size_t STACKSIZE = 1024;
    static constexpr size_t BUFFERSIZE = 128;
    static os::StreamBuffer<uint8_t, BUFFERSIZE> ReceiveBuffer;
    static os::Semaphore FrameAvailable;

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
    CanController(const hal::UsartWithDma & interface,
                  const hal::Gpio & supplyPin,
                  const hal::Gpio & usartTxPin);

    CanController(const CanController &) = delete;
    CanController(CanController &&) = delete;
    CanController& operator=(const CanController&) = delete;
    CanController& operator=(CanController &&) = delete;

    static void CanControllerInterruptHandler(uint8_t);

    void on(void);
    void off(void);
    void triggerFirmwareUpdate(void);
    bool isPerformingFirmwareUpdate(void) const;
    bool wasFirmwareUpdateSuccessful(void) const;

    size_t send(std::string_view, const uint32_t ticksToWait = portMAX_DELAY);
    size_t receive(uint8_t *, size_t, uint32_t ticksToWait = portMAX_DELAY);
    size_t bytesAvailable(void) const;

    void registerReceiveCallback(std::function<void(std::string_view)> );
    void unregisterReceiveCallback(void);
};
}

#endif /* SOURCES_PMD_CANCONTROLLER_H_ */
