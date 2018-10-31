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

#include <string>
#include <string_view>
#include <array>
#include <memory>
#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "os_StreamBuffer.h"
#include "os_Queue.h"
#include "UsartWithDma.h"
#include "Gpio.h"
#include "AT_Parser.h"
#include "Socket.h"

namespace app
{
class ModemDriver final :
    private os::DeepSleepModule
{
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr size_t STACKSIZE = 2048;
    static constexpr size_t BUFFERSIZE = 1024;
    static constexpr size_t ERROR_THRESHOLD = 20;
    static os::StreamBuffer<uint8_t, BUFFERSIZE> InputBuffer;

    std::vector<std::shared_ptr<Socket> > mSockets;

    os::TaskInterruptable mModemTxTask;
    os::TaskInterruptable mParserTask;

    const hal::UsartWithDma& mInterface;
    const hal::Gpio& mModemReset;
    const hal::Gpio& mModemPower;
    const hal::Gpio& mModemSupplyVoltage;

    AT::SendFunction mSend;
    AT::ReceiveFunction mRecv;
    ATParser mParser;
    std::function<void(size_t, size_t)> mUrcCallbackReceive;
    std::function<void(size_t, size_t)> mUrcCallbackClose;

    app::ATCmdOK mATOK;
    app::ATCmdERROR mATERROR;
    app::ATCmdURC mATUUSORF;
    app::ATCmdURC mATUUSORD;
    app::ATCmdURC mATUUPSDD;
    app::ATCmdURC mATUUSOCL;

    size_t mErrorCount = 0;

    void modemTxTaskFunction(const bool&);
    void parserTaskFunction(const bool&);

    void modemOn(void) const;
    void modemOff(void) const;
    void modemReset(void);
    bool modemStartup(void);

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

    std::shared_ptr<Socket> getSocket(Socket::Protocol,
                                      std::string_view ip, std::string_view port);
};
}

#endif /* SOURCES_PMD_MODEMDRIVER_H_ */
