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

#ifndef SOURCES_PMD_MODEMDRIVER_H_
#define SOURCES_PMD_MODEMDRIVER_H_

#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "os_StreamBuffer.h"
#include "UsartWithDma.h"
#include "Gpio.h"
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include "AT_Parser.h"

namespace app
{
class ModemDriver final :
    private os::DeepSleepModule
{
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr size_t STACKSIZE = 4096;
    static constexpr size_t BUFFERSIZE = 512;
    static constexpr size_t ERROR_THRESHOLD = 20;
    static os::StreamBuffer<uint8_t, BUFFERSIZE> InputBuffer;
    static os::StreamBuffer<uint8_t, BUFFERSIZE> SendBuffer;
    static os::StreamBuffer<uint8_t, BUFFERSIZE> ReceiveBuffer;

    os::TaskInterruptable mModemTxTask;
    os::TaskInterruptable mParserTask;

    const hal::UsartWithDma& mInterface;
    const hal::Gpio& mModemReset;
    const hal::Gpio& mModemPower;
    const hal::Gpio& mModemSupplyVoltage;

    AT::SendFunction mSend;
    AT::ReceiveFunction mRecv;
    ATParser mParser;

    std::function<void(std::string_view)> mReceiveCallback;
    size_t mErrorCount = 0;
    std::array<std::shared_ptr<app::ATCmd>, 7> mStartupCommands;

    void modemTxTaskFunction(const bool&);
    void parserTaskFunction(const bool&);

    void modemOn(void) const;
    void modemOff(void) const;
    void modemReset(void);
    bool modemStartup(void);

    bool sendHelloMessage(void);
    void handleError(void);

public:
    ModemDriver(const hal::UsartWithDma & interface,
                const hal::Gpio & resetPin,
                const hal::Gpio & powerPin,
                const hal::Gpio & supplyPin);

    ModemDriver(const ModemDriver &) = delete;
    ModemDriver(ModemDriver &&) = delete;
    ModemDriver& operator=(const ModemDriver&) = delete;
    ModemDriver& operator=(ModemDriver &&) = delete;

    static void ModemDriverInterruptHandler(uint8_t);

    size_t send(std::string_view, const uint32_t ticksToWait = portMAX_DELAY) const;
    size_t receive(uint8_t *, size_t, uint32_t ticksToWait = portMAX_DELAY) const;

    void registerReceiveCallback(std::function<void(std::string_view)> );
    void unregisterReceiveCallback(void);
};
}

#endif /* SOURCES_PMD_MODEMDRIVER_H_ */
