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

using app::ModemDriver;

#define IP "151.236.10.216"
#define PORT "60017"

#define CMD_USOST_BEGIN "AT+USOST=0,\"" IP "\"," PORT ","
#define CMD_USOST "AT+USOST=0,\"" IP "\"," PORT ",1\r"
#define CMD_USOCO "AT+USOCO=0,\"" IP "\"," PORT "\r"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;
static constexpr const size_t SLC_MTU = 32;

os::StreamBuffer<uint8_t, 1024> ModemDriver::InputBuffer;

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
    mModemRxTask("ModemRxTask",
                 ModemDriver::STACKSIZE,
                 os::Task::Priority::VERY_HIGH,
                 [this](const bool& join)
                 {
                     modemRxTaskFunction(join);
                 }),
    mInterface(interface),
    mModemReset(resetPin),
    mModemPower(powerPin),
    mModemSupplyVoltage(supplyPin)
{
    mDataAvailableSemaphore.take(std::chrono::milliseconds(0));
    mInterface.mUsart.enableNonBlockingReceive(ModemDriverInterruptHandler);
}

void ModemDriver::enterDeepSleep(void)
{
    mModemTxTask.join();
    mModemRxTask.join();
}

void ModemDriver::exitDeepSleep(void)
{
    mModemRxTask.start();
    mModemTxTask.start();
}

void ModemDriver::modemTxTaskFunction(const bool& join)
{
    mState = ModemState::STARTMODEM;
    char sendingString[SLC_MTU] = "\r";
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

            case ModemReturnCode::TIMEOUT:
            case ModemReturnCode::FAULT:
                mState = ModemState::STARTMODEM;
                handleError();
                break;
            }

            break;

        case ModemState::TRANSLATEERROR:

            switch (modemSendRecv("AT+CMEE=2\r")) {
            case ModemReturnCode::TIMEOUT:
                mState = ModemState::STARTMODEM;
                break;

            case ModemReturnCode::OK:
                mState = ModemState::CONFIGGPRSCLASS;
                break;

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

            case ModemReturnCode::FAULT:
                os::ThisTask::sleep(std::chrono::seconds(1));
                mState = ModemState::CONFIGGPRSCLASS;
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

            case ModemReturnCode::FAULT:
                os::ThisTask::sleep(std::chrono::seconds(1));
                mState = ModemState::ATTACHGPRS;
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

            case ModemReturnCode::FAULT:
                mState = ModemState::ALLOWUDP;
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

            case ModemReturnCode::FAULT:
                mState = ModemState::SETUDPSOCKET;
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

            case ModemReturnCode::FAULT:
                mState = ModemState::DECLAREHOST;
                handleError();

                break;
            }

            break;

        case ModemState::WAITFORRB:
//                bool ret = STANDARD;
//                if (rxFromCanEnabled) {
//                    memset(sendingString, 0, sizeof(sendingString));
//                    // Trace("Start waiting for Ringbuffer\r\n");
//                    ret = waitforRB(0, sendingString);
//                } else {
//                    // HAL_Delay(1000);
//                }
//
//                switch (ret) {
//                case STANDARD:
//                    if (HAL_GetTick() - timeOfLastUdpSend >= 1000) {
//                        mState = ModemState::SENDHELLO;
//                    } else {
//                        mState = ModemState::CHECKFROMSERVER;
//                    }
//                    break;
//
//                case DATA:
//                    mState = ModemState::SENDDATALENGTH;
//                    break;
//                }
            break;

        case ModemState::SENDHELLO:

            switch (modemSendRecv(CMD_USOST)) {
            case ModemReturnCode::TIMEOUT:
                mState = ModemState::STARTMODEM;
                break;

            case ModemReturnCode::OK:
                mState = ModemState::SENDHELLOSTRING;
                break;

            case ModemReturnCode::FAULT:
                mState = ModemState::SENDHELLO;
                handleError();

                break;
            }

            break;

        case ModemState::SENDDATALENGTH:
            {
                char commandToSend[200];
                memset(commandToSend, 0, sizeof(commandToSend));
                getSendDataLengthCommand(commandToSend, sendingString);

                switch (modemSendRecv(commandToSend)) {
                case ModemReturnCode::TIMEOUT:
                    mState = ModemState::STARTMODEM;
                    break;

                case ModemReturnCode::OK:
                    mState = ModemState::SENDDATASTRING;
                    break;

                case ModemReturnCode::FAULT:
                    mState = ModemState::SENDDATALENGTH;
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

            case ModemReturnCode::FAULT:
                mState = ModemState::SENDHELLOSTRING;
                handleError();

                break;
            }

            break;

        case ModemState::SENDDATASTRING:

            timeOfLastUdpSend = os::Task::getTickCount();

            switch (modemSendRecv(sendingString)) {
            case ModemReturnCode::TIMEOUT:
                mState = ModemState::STARTMODEM;
                break;

            case ModemReturnCode::OK:
                mState = ModemState::CHECKFROMSERVER;
                break;

            case ModemReturnCode::FAULT:
                mState = ModemState::SENDDATASTRING;
                handleError();

                break;
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

            case ModemReturnCode::FAULT:
                mState = ModemState::CHECKFROMSERVER;
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

            case ModemReturnCode::FAULT:
                mState = ModemState::CHECKFROMSERVER;
                handleError();

                break;
            }

            break;

        default:
            break;
        }
    } while (!join);
    // stop code
}

void ModemDriver::modemRxTaskFunction(const bool& join)
{
    do {
        auto && line = readLineFromModem(std::chrono::milliseconds(2500));

        if (line.empty()) {
            handleError();
        } else {
            mDataVector = std::move(line);
            mDataAvailableSemaphore.give();
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
        if (!mDataAvailableSemaphore.take(timeout)) {
            return ModemReturnCode::TIMEOUT;
        }
        ret = parseResponse(std::string_view(reinterpret_cast<const char*>(mDataVector.data()), mDataVector.size()));
    }

    return ModemReturnCode(ret);
}

std::vector<uint8_t> ModemDriver::readLineFromModem(std::chrono::milliseconds timeout)
{
    static constexpr const size_t BUFFERSIZE = 512;
    std::array<uint8_t, BUFFERSIZE> buffer;
    bool insideSubString = false;
    for (auto pos = buffer.begin(); pos != buffer.end(); ) {
        uint8_t newByte = 0;
        if (!InputBuffer.receive(newByte, timeout.count())) {
            return std::vector<uint8_t>();
        }

        //received message contains "
        if (newByte == '"') {
            insideSubString = (insideSubString == false) ? true : false;
        }

        //prompt
        if (newByte == '@') {
            *pos++ = newByte;
            return std::vector<uint8_t>(buffer.begin(), pos);
        }

        const bool terminationFound = (newByte == '\r') || (newByte == '\n') || (newByte == 0);

        if (!insideSubString && terminationFound) {
            *pos++ = 0;
            return std::vector<uint8_t>(buffer.begin(), pos);
        }
        *pos++ = newByte;
    }
    return std::vector<uint8_t>();
}

ModemDriver::ModemReturnCode ModemDriver::parseResponse(std::string_view input)
{
    if (input.find("+USORF") != std::string::npos) {
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
    if (strings.size() == 5) {
        mModemBuffer -= std::stoi(strings[3]);
        //onUdpReceived()
    } else if (strings.size() == 2) {
        mModemBuffer = std::stoi(strings[1]);
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
std::vector<std::string> ModemDriver::splitDataString(std::string_view input)
{
    std::vector<std::string> strings;
    std::string inputString = std::string(input.data(), input.length());

    std::istringstream iss(inputString);
    std::string s;
    for (auto i = 0; i < 4 && getline(iss, s, ','); i++) {
        strings.push_back(s);
    }
    if (getline(iss, s)) {strings.push_back(s); }
    return strings;
}

void ModemDriver::onUdpReceived(uint8_t        socket,
                                const char*    host,
                                uint16_t       port,
                                const uint8_t* data,
                                unsigned int   length)
{
    if (data != NULL) {
        Trace(ZONE_INFO, "UDP packet received: ");
        for (size_t i = 0; i < length; i++) {
            if (i % 16 == 0) {
                Trace(ZONE_INFO, "\r\n  0x%04x:", i);
            }
            Trace(ZONE_INFO, " %02x", data[i]);
        }
        Trace(ZONE_INFO, "\r\n");
    }

    if (length == 0) {
        return;
    }
}

bool ModemDriver::waitforRB(unsigned int delay, char* returnstring)
{
    uint8_t data;
    auto ret = InputBuffer.receive(data, delay);
    *returnstring = static_cast<char>(data);
    return ret;
}

void ModemDriver::getSendDataLengthCommand(char* outputstring, char const* const dataStringToSend)
{
    char command[200] = CMD_USOST_BEGIN;

    char string[10];
    memset(string, 0, 10);

    char tempString[100];
    std::memcpy(tempString, dataStringToSend, strlen(dataStringToSend));

    char* endptr;
    endptr = strstr(tempString, "\r");
    endptr++;
    unsigned int dataStringLength = (unsigned int)(endptr - dataStringToSend);

    if (dataStringLength > SLC_MTU) {
        Trace(ZONE_INFO, "ERROR, unexpected length of CAN TO GSM string\n");
    }

    sprintf(string, "%d", dataStringLength % SLC_MTU);

    strcat(command, string);
    strcat(command, "\r");

    strcpy(outputstring, command);
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
