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

#include "ModemDriver.h"
#include "LockGuard.h"
#include "trace.h"
#include <cstring>
#include <sstream>
#include <string>
#include <algorithm>
#include <ostream>

using app::ModemDriver;

#define IP "151.236.10.216"
#define PORT "60017"

#define CMD_USOST_BEGIN "AT+USOST=0,\"" IP "\"," PORT ","
#define CMD_USOST "AT+USOST=0,\"" IP "\"," PORT ",1\r"
#define CMD_USOCO "AT+USOCO=0,\"" IP "\"," PORT "\r"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;
static constexpr const size_t SLC_MTU = 32;

os::StreamBuffer<uint8_t, 1024> ModemDriver::InputBuffer;
os::StreamBuffer<uint8_t, 1024> ModemDriver::OutputBuffer;

os::Semaphore ModemDriver::InputAvailable;

void ModemDriver::ModemDriverInterruptHandler(uint8_t data)
{
    static bool insideSubString = false;

    //received message contains "
    if (data == '"') {
        insideSubString = (insideSubString == false) ? true : false;
    }

    InputBuffer.sendFromISR(data);

    const bool terminationFound = (data == '\r') || (data == '\n') || (data == 0);
    const bool promptFound = (data == '@');
    if ((!insideSubString && terminationFound) || (promptFound)) {
        InputAvailable.giveFromISR();
    }
}

ModemDriver::ModemDriver(const hal::UsartWithDma& interface,
                         const hal::Gpio&         resetPin,
                         const hal::Gpio&         powerPin,
                         const hal::Gpio&         supplyPin) :
    os::DeepSleepModule(),
    mModemTxTask("ModemTxTask",
                 ModemDriver::STACKSIZE,
                 os::Task::Priority::HIGH,
                 [this](const bool& join)
                 {
                     modemTxTaskFunction(join);
                 }),
    mInterface(interface),
    mModemReset(resetPin),
    mModemPower(powerPin),
    mModemSupplyVoltage(supplyPin)
{
    mInterface.mUsart.enableNonBlockingReceive(ModemDriverInterruptHandler);
}

void ModemDriver::enterDeepSleep(void)
{
    mModemTxTask.join();
}

void ModemDriver::exitDeepSleep(void)
{
    mModemTxTask.start();
}

void ModemDriver::modemTxTaskFunction(const bool& join)
{
    mState = ModemState::STARTMODEM;
    size_t numberOfBytesToSend = 0;
    uint32_t timeOfLastUdpSend = 0;
    do {
        switch (mState) {
        case ModemState::STARTMODEM:
            modemReset();
            mErrorCount = 0;

            switch (modemSendRecv("ATE0V1\r")) {
            case ModemReturnCode::OK:
                mState = ModemState::TRANSLATEERROR;
                break;

            case ModemReturnCode::TRY_AGAIN:
            case ModemReturnCode::TIMEOUT:
            case ModemReturnCode::FAULT:
                handleError();
                break;
            }

            break;

        case ModemState::TRANSLATEERROR:

            switch (modemSendRecv("AT+CMEE=2\r")) {
            case ModemReturnCode::OK:
                mState = ModemState::CONFIGGPRSCLASS;
                break;

            case ModemReturnCode::TIMEOUT:
            case ModemReturnCode::TRY_AGAIN:
            case ModemReturnCode::FAULT:
                mState = ModemState::STARTMODEM;
                handleError();
                break;
            }

            break;

        case ModemState::CONFIGGPRSCLASS:

            switch (modemSendRecv("AT+CGCLASS=\"B\"\r")) {
            case ModemReturnCode::TIMEOUT:
                mState = ModemState::STARTMODEM;
                break;

            case ModemReturnCode::OK:
                mState = ModemState::ATTACHGPRS;
                break;

            case ModemReturnCode::TRY_AGAIN:
            case ModemReturnCode::FAULT:
                os::ThisTask::sleep(std::chrono::seconds(1));
                handleError();

                break;
            }

            break;

        case ModemState::ATTACHGPRS:

            switch (modemSendRecv("AT+CGATT=1\r")) {
            case ModemReturnCode::TIMEOUT:
                mState = ModemState::STARTMODEM;
                break;

            case ModemReturnCode::OK:
                mState = ModemState::ALLOWUDP;
                break;

            case ModemReturnCode::TRY_AGAIN:
            case ModemReturnCode::FAULT:
                os::ThisTask::sleep(std::chrono::seconds(1));
                handleError();

                break;
            }

            break;

        case ModemState::ALLOWUDP:

            switch (modemSendRecv("AT+UPSDA=0,3\r")) {
            case ModemReturnCode::TIMEOUT:
                mState = ModemState::STARTMODEM;
                break;

            case ModemReturnCode::OK:
                mState = ModemState::SETUDPSOCKET;
                break;

            case ModemReturnCode::TRY_AGAIN:
            case ModemReturnCode::FAULT:
                handleError();

                break;
            }

            break;

        case ModemState::SETUDPSOCKET:

            switch (modemSendRecv("AT+USOCR=17\r")) {
            case ModemReturnCode::TIMEOUT:
                mState = ModemState::STARTMODEM;
                break;

            case ModemReturnCode::OK:
                mState = ModemState::DECLAREHOST;
                break;

            case ModemReturnCode::TRY_AGAIN:
            case ModemReturnCode::FAULT:
                handleError();

                break;
            }

            break;

        case ModemState::DECLAREHOST:

            switch (modemSendRecv(CMD_USOCO)) {
            case ModemReturnCode::TIMEOUT:
                mState = ModemState::STARTMODEM;
                break;

            case ModemReturnCode::OK:
                mState = ModemState::WAITFORRB;
                break;

            case ModemReturnCode::TRY_AGAIN:
            case ModemReturnCode::FAULT:
                handleError();

                break;
            }

            break;

        case ModemState::WAITFORRB:

            if (OutputBuffer.isEmpty()) {
                if (os::Task::getTickCount() - timeOfLastUdpSend >= 1000) {
                    mState = ModemState::SENDHELLO;
                } else {
                    mState = ModemState::CHECKFROMSERVER;
                }
            } else {
                numberOfBytesToSend = OutputBuffer.bytesAvailable();
                mState = ModemState::SENDDATALENGTH;
            }
            break;

        case ModemState::SENDHELLO:

            switch (modemSendRecv(CMD_USOST)) {
            case ModemReturnCode::TIMEOUT:
                mState = ModemState::STARTMODEM;
                break;

            case ModemReturnCode::OK:
                mState = ModemState::SENDHELLOSTRING;
                break;

            case ModemReturnCode::TRY_AGAIN:
            case ModemReturnCode::FAULT:
                handleError();

                break;
            }

            break;

        case ModemState::SENDDATALENGTH:
            {
                std::string cmd = CMD_USOST_BEGIN + std::to_string(numberOfBytesToSend) + "\r";

                switch (modemSendRecv(cmd)) {
                case ModemReturnCode::TIMEOUT:
                    mState = ModemState::STARTMODEM;
                    break;

                case ModemReturnCode::OK:
                    mState = ModemState::SENDDATASTRING;
                    break;

                case ModemReturnCode::TRY_AGAIN:
                case ModemReturnCode::FAULT:
                    handleError();

                    break;
                }

                break;
            }

        case ModemState::SENDHELLOSTRING:

            timeOfLastUdpSend = os::Task::getTickCount();

            switch (modemSendRecv("\r")) {
            case ModemReturnCode::TIMEOUT:
                mState = ModemState::STARTMODEM;
                break;

            case ModemReturnCode::OK:
                mState = ModemState::CHECKFROMSERVER;
                break;

            case ModemReturnCode::TRY_AGAIN:
            case ModemReturnCode::FAULT:
                handleError();

                break;
            }

            break;

        case ModemState::SENDDATASTRING:
            {
                timeOfLastUdpSend = os::Task::getTickCount();
                std::string output(numberOfBytesToSend, '\x00');
                OutputBuffer.receive(output.data(), output.size(), 1);

                switch (modemSendRecv(output)) {
                case ModemReturnCode::TIMEOUT:
                    mState = ModemState::STARTMODEM;
                    break;

                case ModemReturnCode::OK:
                    mState = ModemState::CHECKFROMSERVER;
                    break;

                case ModemReturnCode::TRY_AGAIN:
                case ModemReturnCode::FAULT:
                    handleError();
                    break;
                }
            }

            break;

        case ModemState::CHECKFROMSERVER:

            switch (modemSendRecv("AT+USORF=0,0\r")) {
            case ModemReturnCode::TIMEOUT:
                mState = ModemState::STARTMODEM;
                break;

            case ModemReturnCode::OK:
                if (mModemBuffer > 0) {
                    mState = ModemState::RECEIVEFROMSERVER;
                } else {
                    mState = ModemState::WAITFORRB;
                }
                break;

            case ModemReturnCode::TRY_AGAIN:
            case ModemReturnCode::FAULT:
                handleError();

                break;
            }

            break;

        case ModemState::RECEIVEFROMSERVER:

            switch (modemSendRecv("AT+USORF=0,40\r")) {
            case ModemReturnCode::TIMEOUT:
                mState = ModemState::STARTMODEM;
                break;

            case ModemReturnCode::OK:
                mState = ModemState::CHECKFROMSERVER;
                break;

            case ModemReturnCode::TRY_AGAIN:
            case ModemReturnCode::FAULT:
                handleError();

                break;
            }

            break;
        }
    } while (!join);
}

void ModemDriver::modemOn(void) const
{
    mModemSupplyVoltage = true;
}

void ModemDriver::modemOff(void) const
{
    mModemReset = false;
    mModemPower = false;
    mModemSupplyVoltage = false;
}

void ModemDriver::modemReset(void) const
{
    modemOff();
    os::ThisTask::sleep(std::chrono::milliseconds(1000));
    modemOn();
    os::ThisTask::sleep(std::chrono::milliseconds(1000));
}

ModemDriver::ModemReturnCode ModemDriver::modemSendRecv(std::string_view str, std::chrono::milliseconds timeout)
{
    ModemReturnCode ret = ModemReturnCode::TRY_AGAIN;

    mInterface.send(str);
    while (ret == ModemReturnCode::TRY_AGAIN) {
        if (!InputAvailable.take(timeout)) {
            return ModemReturnCode::TIMEOUT;
        }
        auto length = InputBuffer.bytesAvailable();
        std::string input(length, '\x00');
        if (!InputBuffer.receive(input.data(), input.size(), timeout.count())) {
            return ModemReturnCode::TIMEOUT;
        }
        ret = parseResponse(std::string_view(reinterpret_cast<const char*>(input.data()), input.size()));
    }

    return ModemReturnCode(ret);
}

ModemDriver::ModemReturnCode ModemDriver::parseResponse(std::string_view input)
{
    if (input.find("+USORF") != std::string::npos) {
        handleDataReception(input);
        return ModemReturnCode::TRY_AGAIN;
    } else if (input.find("+UUSORF") != std::string::npos) {
        handleDataReception(input);
        return ModemReturnCode::TRY_AGAIN;
    } else if (input.find("OK") != std::string::npos) {
        return ModemReturnCode::OK;
    } else if (input.find("ERROR") != std::string::npos) {
        return ModemReturnCode::FAULT;
    } else if (input.find("@") != std::string::npos) {
        return ModemReturnCode::OK;
    } else {
        Trace(ZONE_ERROR, "Couldn't parse \"%s\"\r\n", input);
        return ModemReturnCode::TRY_AGAIN;
    }
}

void ModemDriver::handleDataReception(std::string_view input)
{
    auto strings = splitDataString(input);
    if (strings.size() == 6) {
        mModemBuffer -= std::stoi(strings[4]);
        auto data = strings[5];
        if (data.length() > 2) {
            auto first = data.find_first_of('"') + 1;
            auto last = data.find_last_of('"');
            mDataString = std::string(data.c_str() + first, last - first);
        }
    } else if (strings.size() == 2) {
        mModemBuffer = std::stoi(strings[2]);
    } else {
        Trace(ZONE_ERROR, "Malformed GSM data \r\n");
    }
}

/* Parses the response to an AT+USORF command,
 * the response is composed of 5 pieces, separated by delimeter ",",
 * +USORF: SOCKET_IDX,"IP_ADDRESS",PORT,DATA_LEN,"DATA".
 * It is also possible that the response is only 2 pieces
 * +USORF: SOCKET_IDX,DATA_LEN
 * This indicates only how much data is in the buffer*/
const std::vector<std::string> ModemDriver::splitDataString(std::string_view input) const
{
    std::vector<std::string> strings;
    std::string inputString = std::string(input.data(), input.length());
    std::istringstream iss(inputString);
    std::string s;
    if (getline(iss, s, ':')) {strings.push_back(s); }
    for (auto i = 0; i < 4 && getline(iss, s, ','); i++) {
        strings.push_back(s);
    }
    if (getline(iss, s)) {strings.push_back(s); }
    return strings;
}

void ModemDriver::handleError(void)
{
    Trace(ZONE_ERROR, "Error in current State\n");

    if (mErrorCount == 10) {
        mState = ModemState::STARTMODEM;
        mErrorCount = 0;
    }
    mErrorCount++;
}
