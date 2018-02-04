/* Copyright (C) 2015  Nils Weiss, Daniel Tatzel, Markus Wildgruber
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
#include "Semaphore.h"
#include "os_Queue.h"
#include "UsartWithDma.h"
#include "Gpio.h"
#include <string>
#include <string_view>
#include <vector>
#include <sstream>

namespace app
{
struct ModemDriverTester;

class ModemDriver final :
    private os::DeepSleepModule
{
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr uint32_t STACKSIZE = 4096;
    static constexpr size_t BUFFERSIZE = 512;
    static os::StreamBuffer<uint8_t, BUFFERSIZE> InputBuffer;
    static os::StreamBuffer<uint8_t, BUFFERSIZE> OutputBuffer;

    enum class ModemState
    {
        STARTMODEM = 0,
        TRANSLATEERROR = 1,
        CONFIGGPRSCLASS = 2,
        ATTACHGPRS = 3,
        ALLOWUDP = 4,
        SETUDPSOCKET = 5,
        DECLAREHOST = 6,
        WAITFORRB = 7,
        SENDHELLO = 8,
        SENDDATALENGTH = 9,
        SENDHELLOSTRING = 10,
        SENDDATASTRING = 11,
        CHECKFROMSERVER = 12,
        RECEIVEFROMSERVER = 13,
    };

    enum class ParseResult
    {
        USORF,
        UUSORD,
        USOST,
        OK,
        ERROR,
        PROMPT,
        NEWLINE,
        CARRIAGE_RETURN,
        UNKNOWN
    };

    enum class ModemReturnCode
    {
        TIMEOUT = -1,
        OK = 0,
        FAULT = 1,
        TRY_AGAIN = 2
    };

    os::TaskInterruptable mModemTxTask;
    std::string mDataString;

    const hal::UsartWithDma& mInterface;
    const hal::Gpio& mModemReset;
    const hal::Gpio& mModemPower;
    const hal::Gpio& mModemSupplyVoltage;

    size_t mErrorCount = 0;
    int mModemBuffer = 0;
    ModemState mState = ModemState::STARTMODEM;
    std::array<char, BUFFERSIZE> mLine;

    void modemTxTaskFunction(const bool&);

    void modemOn(void) const;
    void modemOff(void) const;
    void modemReset(void);

    ModemReturnCode modemStartup(void);
    ModemReturnCode sendHelloMessage(void);

    ModemReturnCode modemSendRecv(std::string_view, std::chrono::milliseconds timeout = std::chrono::milliseconds(8000));
    ParseResult parseResponse(std::string_view input) const;
    ModemReturnCode interpretResponse(const ParseResult& response, std::string_view input);

    void handleDataReception(std::string_view input);
    const std::vector<std::string> splitDataString(std::string_view input) const;
    std::string_view readLineFromInput(std::chrono::milliseconds timeout);
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

    friend struct ModemDriverTester;
};
#if defined(UNITTEST)
class ModemDriverTester
{
    ModemDriver& mTestee;
public:

    template<class ... Args>
    decltype(auto) splitDataString(Args && ... args)
    {
        return mTestee.splitDataString(std::forward<Args>(args) ...);
    }

    template<class ... Args>
    decltype(auto) modemSendRecv(Args && ... args)
    {
        return static_cast<int>(mTestee.modemSendRecv(std::forward<Args>(args) ...));
    }

    decltype(auto) getDataString(void)
    {
        return mTestee.mDataString;
    }

    ModemDriverTester(ModemDriver& testee) :
        mTestee(testee){}
private:
};
#endif
}

#endif /* SOURCES_PMD_MODEMDRIVER_H_ */
