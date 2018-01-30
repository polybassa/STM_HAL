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
    mModemDriverTask(
                     "ModemDriverTask",
                     ModemDriver::STACKSIZE,
                     os::Task::Priority::HIGH,
                     [this](const bool& join)
                     {
                         modemDriverTaskFunction(join);
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
    mModemDriverTask.join();
}

void ModemDriver::exitDeepSleep(void)
{
    mModemDriverTask.start();
}

void ModemDriver::modemDriverTaskFunction(const bool& join)
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

ModemDriver::ModemReturnCode ModemDriver::modemSendRecv(std::string_view str)
{
    char somebuf[81];
    int ret = 2;

    mInterface.send(str);

    while (ret == 2) {
        std::memset(somebuf, 0, sizeof(somebuf));

        //wait til a full message arrived
        ret = readLineFromRingBuffer(somebuf, sizeof(somebuf));
        Trace(ZONE_INFO, "> %s\r\n", somebuf);

        if (ret == -1) {
            Trace(ZONE_INFO, "TIMEOUT: (((%s)))\r\n", somebuf);
            return ModemReturnCode::TIMEOUT;
        }

        if (str == "AT+USORF") {
            ret = ParseResponseLine_USORF(somebuf);
        } else {
            ret = ParseResponseLine(somebuf);
        }
    }

    return ModemReturnCode(ret);
}

int ModemDriver::readLineFromRingBuffer(char* string, int size)
{
    char linebufString[81];
    int linebuf_idx = 0;
    memset(linebufString, 0, sizeof(linebufString));
    int inside_string = 0;

    uint8_t r = 0;
    while (1) {
        if (!InputBuffer.receive(r, 25000)) {
            return -1;
        }

        //received message contains "
        if (r == '"') {
            if (inside_string) { inside_string = 0; } else { inside_string = 1; }
        }

        //prompt
        if (r == '@') {
            linebufString[linebuf_idx++] = r;
            strcpy(string, linebufString);
            return strlen(string);
        }

        if (!inside_string && ((r == '\r') || (r == '\n') || (r == 0))) {
            linebufString[linebuf_idx] = 0;
            if (linebuf_idx >= size) {
                linebuf_idx = 0;
                return 0;
            }
            linebuf_idx = 0;
            strcpy(string, linebufString);
            //Trace(ZONE_INFO,"%s\n",string);
            return strlen(string);
        }

        linebufString[linebuf_idx++] = r;

        if (linebuf_idx >= sizeof(linebufString)) {
            linebuf_idx = 0;
            return 0;
        }
    }
}

int ModemDriver::ParseResponseLine(char* answerbuf)
{
    if (memcmp(answerbuf, "OK", 2) == 0) {
        //OK is received
        return 0;
    } else if (memcmp(answerbuf, "+CME ERROR", 10) == 0) {
        //ERROR is received
        return 1;
    } else if (memcmp(answerbuf, "ERROR", 5) == 0) {
        //ERROR is received
        return 1;
    } else if (memcmp(answerbuf, "@", 1) == 0) {
        //prompt
        return 0;
    } else {
        //ECHO or nothing
        return 2;
    }
}

int ModemDriver::ParseResponseLine_USORF(char* answerbuf)
{
    if (memcmp(answerbuf, "OK", 2) == 0) {
        //OK is received
        return 0;
    } else if (memcmp(answerbuf, "ERROR", 5) == 0) {
        //ERROR is received
        return 1;
    } else if (memcmp(answerbuf, "+USORF", 6) == 0) {
        //some answerstuff
        handle_USORF(answerbuf);
        return 2;
    } else {
        //ECHO or nothing
        //send the received message to the can controller
        return 2;
    }
}

/* Parses the response to an AT+USORF command,
 * the response is composed of 5 pieces, separated by delimeter ",",
 * +USORF: SOCKET_IDX,"IP_ADDRESS",PORT,DATA_LEN,"DATA".
 * It is also possible that the response is only 2 pieces
 * +USORF: SOCKET_IDX,DATA_LEN
 * This indicates only how much data is in the buffer*/
void ModemDriver::handle_USORF(char* string)
{
    char delimiter[] = ",";
    char* ptr;

    int i = 0;
    char data[80];

    //Trace(ZONE_INFO,"Receive: %s\n", string);

    memset(data, 0, sizeof(data));

    int socket = -1;
    char* addrstr = NULL;
    int port = -1;
    char* lengthstr = NULL;
    char* datastr = NULL;

    ptr = strtok(string, delimiter);

    while (ptr != NULL) {
        // naechsten Abschnitt erstellen
        if (i == 0) {
            socket = strtol(ptr + 8, NULL, 10);
        } else if (i == 1) {
            if (ptr[0] == '"') {
                addrstr = ptr;
            } else {
                lengthstr = ptr;
                break;
            }
        } else if (i == 2) {
            port = strtol(ptr, NULL, 10);
        } else if (i == 3) {
            lengthstr = ptr;
        } else if (i == 4) {
            datastr = ptr;
            break;
        }

        ptr = strtok(NULL, delimiter);
        i++;
    }

    if (lengthstr == NULL) {
        /* Malformed response, contains no length! */
        Trace(ZONE_INFO, "Malformed response to USORF: %s\r\n", string);
        return;
    }

    /* Parse the length parameter */
    strcpy(data, lengthstr);
    int length = strtol(data, NULL, 10);

    if (datastr != NULL) {
        /* There is data to be read */
        memset(data, 0, sizeof(data));
        memcpy(data, ++datastr, length); // skip the quotation mark

        onUdpReceived(socket, addrstr, port, (uint8_t*)data, length);
        mModemBuffer -= length; // this estimate can be wrong, USORF should be called again to check if the buffer is still full
    } else {
        /* Received indication of how much data is in the modem buffer */
        mModemBuffer = length;
    }
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

//    uint8_t packetType = data[0];
//
//    if ((packetType == 1) || (packetType == '/')) {
//        /* This kind of packet is handled internally by MaCo, don't forward. */
//        if (length <= 1) {
//            return; // Empty command
//        }
//        specialCommand_t cmd = data[1];
//        handleSpecialCommand(cmd, data + 2, length - 2);
//    } else {
//        /* Anything else is forwarded to SecCo */
//        sendString(&huart2, (const char*)data, length);
//    }
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

    char* endptr;
    endptr = strstr(dataStringToSend, "\r");
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
