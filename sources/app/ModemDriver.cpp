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

using app::ModemDriver;

#define IP "195.34.89.241"
#define PORT "7"

#define CMD_USOST_BEGIN "AT+USOST=0,\"" IP "\"," PORT ","
#define CMD_USOST "AT+USOST=0,\"" IP "\"," PORT ",6\r"
#define CMD_USOCO "AT+USOCO=0,\"" IP "\"," PORT "\r"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

os::StreamBuffer<uint8_t, ModemDriver::BUFFERSIZE> ModemDriver::InputBuffer;
os::StreamBuffer<uint8_t, ModemDriver::BUFFERSIZE> ModemDriver::OutputBuffer;

void ModemDriver::ModemDriverInterruptHandler(uint8_t data)
{
    InputBuffer.sendFromISR(data);
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
    do {
        if (modemStartup() != ModemReturnCode::OK) {
            continue;
        }

        mState = ModemState::WAITFORRB;
        std::string output;
        uint32_t timeOfLastUdpSend = 0;

        while (mState >= ModemState::WAITFORRB) {
            switch (mState) {
            case ModemState::WAITFORRB:
                if (OutputBuffer.isEmpty()) {
                    if (os::Task::getTickCount() - timeOfLastUdpSend >= 1000) {
                        if (sendHelloMessage() != ModemReturnCode::OK) {
                            handleError();
                        } else {
                            timeOfLastUdpSend = os::Task::getTickCount();
                        }
                    } else {
                        // wait a second for new input data

                        modemSendRecv("", std::chrono::milliseconds(1000));
                        if (mModemBuffer > 0) {
                            mState = ModemState::RECEIVEFROMSERVER;
                        }
                    }
                } else {
                    mState = ModemState::SENDDATALENGTH;
                }
                break;

            case ModemState::SENDDATALENGTH:
                {
                    auto numberOfBytesToSend = OutputBuffer.bytesAvailable();
                    output = std::string(numberOfBytesToSend, '\x00');
                    OutputBuffer.receive(output.data(), output.size(), 1);

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

            case ModemState::SENDDATASTRING:
                timeOfLastUdpSend = os::Task::getTickCount();

                switch (modemSendRecv(output)) {
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

            case ModemState::RECEIVEFROMSERVER:

                switch (modemSendRecv("AT+USORF=0," + std::to_string(mModemBuffer) + "\r")) {
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
            }
        }
    } while (!join);
}

ModemDriver::ModemReturnCode ModemDriver::sendHelloMessage(void)
{
    if (modemSendRecv(CMD_USOST) != ModemReturnCode::OK) {
        return ModemReturnCode::FAULT;
    }

    if (modemSendRecv("HELLO\r") != ModemReturnCode::OK) {
        return ModemReturnCode::FAULT;
    }
    return ModemReturnCode::OK;
}

ModemDriver::ModemReturnCode ModemDriver::modemStartup(void)
{
    modemReset();

    if (modemSendRecv("ATE0V1\r", std::chrono::seconds(10)) != ModemReturnCode::OK) {
        return ModemReturnCode::FAULT;
    }
    InputBuffer.reset();

    if (modemSendRecv("AT+CMEE=2\r") != ModemReturnCode::OK) {
        return ModemReturnCode::FAULT;
    }
    os::ThisTask::sleep(std::chrono::seconds(5));
    InputBuffer.reset();

    if (modemSendRecv("AT+CGCLASS=\"B\"\r", std::chrono::seconds(10)) != ModemReturnCode::OK) {
        return ModemReturnCode::FAULT;
    }
    InputBuffer.reset();

    if (modemSendRecv("AT+CGATT=1\r", std::chrono::seconds(10)) != ModemReturnCode::OK) {
        return ModemReturnCode::FAULT;
    }

    if (modemSendRecv("AT+UPSDA=0,3\r") != ModemReturnCode::OK) {
        return ModemReturnCode::FAULT;
    }
    if (modemSendRecv("AT+USOCR=17\r") != ModemReturnCode::OK) {
        return ModemReturnCode::FAULT;
    }

    if (modemSendRecv(CMD_USOCO) != ModemReturnCode::OK) {
        return ModemReturnCode::FAULT;
    }

    return ModemReturnCode::OK;
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

void ModemDriver::modemReset(void)
{
    modemOff();
    InputBuffer.reset();
    mErrorCount = 0;
    os::ThisTask::sleep(std::chrono::milliseconds(1000));
    modemOn();
    os::ThisTask::sleep(std::chrono::milliseconds(1000));
}

std::string_view ModemDriver::readLineFromInput(std::chrono::milliseconds timeout)
{
    bool insideSubString = false;
    uint8_t data;
    size_t lineEndIndex = 0;
    while (InputBuffer.receive(data, timeout.count()) && lineEndIndex < mLine.size()) {
        mLine[lineEndIndex++] = static_cast<char>(data);

        if (data == '"') {
            insideSubString = (insideSubString == false) ? true : false;
        }

        const bool terminationFound = (data == '\r') || (data == '\n') || (data == 0) || (data == '@');

        if (!insideSubString && terminationFound) {
            break;
        }
    }
    return std::string_view(mLine.data(), lineEndIndex);
}

ModemDriver::ModemReturnCode ModemDriver::modemSendRecv(std::string_view str, std::chrono::milliseconds timeout)
{
    Trace(ZONE_VERBOSE, "Send: %s\r\n", str.data());
    mInterface.send(str);

    ModemReturnCode ret = ModemReturnCode::TRY_AGAIN;
    while (ret == ModemReturnCode::TRY_AGAIN) {
        std::string_view response = readLineFromInput(timeout);
        if (response.length() == 0) {
            return ModemReturnCode::TIMEOUT;
        }

        auto parserResult = parseResponse(response);
        ret = interpretResponse(parserResult, response);
    }
    Trace(ZONE_VERBOSE, " Return: %d\r\n", static_cast<size_t>(ret));

    return ModemReturnCode(ret);
}

ModemDriver::ModemReturnCode ModemDriver::interpretResponse(const ParseResult& response, std::string_view input)
{
    switch (response) {
    case ParseResult::ERROR:
        return ModemReturnCode::FAULT;

    case ParseResult::UUSORD:
        handleDataReception(input);

    case ParseResult::PROMPT:
    case ParseResult::OK:
        return ModemReturnCode::OK;

    case ParseResult::USORF:
        handleDataReception(input);

    case ParseResult::USOST:
    case ParseResult::NEWLINE:
    case ParseResult::CARRIAGE_RETURN:
    case ParseResult::UNKNOWN:
        return ModemReturnCode::TRY_AGAIN;

    default:
        return ModemReturnCode::TRY_AGAIN;
    }
}

ModemDriver::ParseResult ModemDriver::parseResponse(std::string_view input) const
{
    if (input.find("+USORF") != std::string_view::npos) {
        return ParseResult::USORF;
    } else if (input.find("+UUSORD") != std::string_view::npos) {
        return ParseResult::UUSORD;
    } else if (input.find("+USOST") != std::string_view::npos) {
        return ParseResult::USOST;
    } else if (input.find("OK") != std::string_view::npos) {
        return ParseResult::OK;
    } else if (input.find("ERROR") != std::string_view::npos) {
        return ParseResult::ERROR;
    } else if (input.find("@") != std::string_view::npos) {
        return ParseResult::PROMPT;
    } else if ((*input.data() == '\r') && (input.length() == 1)) {
        return ParseResult::CARRIAGE_RETURN;
    } else if ((*input.data() == '\n') && (input.length() == 1)) {
        return ParseResult::NEWLINE;
    } else {
        Trace(ZONE_ERROR, "Couldn't parse \r\n%s\r\n", input.data());
        return ParseResult::UNKNOWN;
    }
}

void ModemDriver::handleDataReception(std::string_view input)
{
    auto strings = splitDataString(input);

    if (strings.size() == 6) {
        size_t result = static_cast<size_t>(std::atoi(strings[4].data()));
        result %= BUFFERSIZE;
        mModemBuffer -= result;

        auto data = strings[5];
        if (data.length() > 2) {
            auto first = data.find_first_of('"') + 1;
            auto last = data.find_last_of('"');

            mDataString = std::string(data.data() + first, last - first);
            Trace(ZONE_INFO, "GOT data %s \r\n", mDataString.c_str());
        }
    } else if (strings.size() == 3) {
        size_t result = static_cast<size_t>(std::atoi(strings[2].data()));
        result %= BUFFERSIZE;
        mModemBuffer = result;
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
const std::vector<std::string_view> ModemDriver::splitDataString(std::string_view input) const
{
    std::vector<std::string_view> strings;

    size_t startindex = 0;
    size_t endindex = input.find(':');
    if (endindex == std::string_view::npos) {
        return strings;
    }
    strings.emplace_back(input.data() + startindex, endindex);

    for (auto i = 0; i < 4; i++) {
        startindex = endindex + 1;
        endindex = input.find(',', startindex);
        if (endindex == std::string_view::npos) {
            strings.emplace_back(input.data() + startindex, input.length() - startindex);
            return strings;
        }
        strings.emplace_back(input.data() + startindex, endindex - startindex);
    }
    endindex++;
    strings.emplace_back(input.data() + endindex, input.length() - endindex);

    return strings;
}

void ModemDriver::handleError(void)
{
    Trace(ZONE_ERROR, "Error in current State\r\n");

    if (mErrorCount == 20) {
        mState = ModemState::STARTMODEM;
        mErrorCount = 0;
    }
    mErrorCount++;
}
