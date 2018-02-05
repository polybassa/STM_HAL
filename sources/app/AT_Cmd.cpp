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

#include "AT_Cmd.h"
#include "trace.h"

using app::AtCmd;
using app::BasicAtCmd;
using app::ReceiveAtCmd;
using app::SendAtCmd;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE; // | ZONE_INFO;

std::array<char, AtCmd::BUFFERSIZE> AtCmd::ReceiveBuffer;

std::string_view AtCmd::readLine(void) const
{
    uint8_t data;
    size_t lineEndIndex = 0;
    while (mReceive(data, mTimeout) && lineEndIndex < ReceiveBuffer.size()) {
        ReceiveBuffer[lineEndIndex++] = static_cast<char>(data);
        const bool terminationFound = (data == '\r') || (data == '\n') || (data == 0);

        if (terminationFound) {
            break;
        }
    }
    return std::string_view(ReceiveBuffer.data(), lineEndIndex);
}

AtCmd::ParseResult AtCmd::parseResponse(std::string_view input) const
{
    if (input.find("+USORF:") != std::string_view::npos) {
        return ParseResult::USORF;
    } else if (input.find("+UUSORF:") != std::string_view::npos) {
        return ParseResult::UUSORF;
    } else if (input.find("+UUSORD:") != std::string_view::npos) {
        return ParseResult::UUSORD;
    } else if (input.find("+USOST:") != std::string_view::npos) {
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
        Trace(ZONE_INFO, "Couldn't parse \r\n%s\r\n", std::string(input.data(), input.length()).c_str());
        return ParseResult::UNKNOWN;
    }
}

bool AtCmd::waitForErrorOrOk(void) const
{
    while (true) {
        std::string_view response = readLine();
        if (response.length() == 0) {
            return false;
        }
        auto result = parseResponse(response);
        if (result == AtCmd::ParseResult::OK) {
            Trace(ZONE_VERBOSE, "OK\r\n");
            return true;
        }
        if (result == AtCmd::ParseResult::ERROR) {
            Trace(ZONE_ERROR, "ERROR got\r\n");
            return false;
        }
    }
}

bool BasicAtCmd::process(void) const
{
    Trace(ZONE_VERBOSE, "Process %s\n", std::string(mRequest.data(), mRequest.length()).c_str());
    if (mSend(mRequest, mTimeout) != mRequest.length()) {
        return false;
    }

    return waitForErrorOrOk();
}

std::string_view SendAtCmd::readUntilPrompt(void) const
{
    uint8_t data;
    size_t lineEndIndex = 0;
    while (mReceive(data, mTimeout) && lineEndIndex < ReceiveBuffer.size()) {
        ReceiveBuffer[lineEndIndex++] = static_cast<char>(data);
        const bool terminationFound = (data == '@');
        Trace(ZONE_INFO, "Read: 0x%02X \r\n", data);

        if (terminationFound) {
            break;
        }
    }
    return std::string_view(ReceiveBuffer.data(), lineEndIndex);
}

bool SendAtCmd::process(std::string_view input) const
{
    Trace(ZONE_VERBOSE, "Process send %s\r\n", std::string(input.data(), input.length()).c_str());

    std::string cmd = AT_CMD_USOST + std::to_string(input.length()) + "\r";

    if (mSend(cmd, mTimeout) != cmd.length()) {
        return false;
    }
    Trace(ZONE_INFO, "Send %s\n", cmd.c_str());

    std::string_view response = readUntilPrompt();
    if (response.length() == 0) {
        return false;
    }

    Trace(ZONE_INFO, "got prompt\r\n");

    if (mSend(input, mTimeout) != input.length()) {
        return false;
    }

    return waitForErrorOrOk();
}

bool ReceiveAtCmd::process(std::string& receivedData, bool poll) const
{
    Trace(ZONE_VERBOSE, "Process Receive\r\n");

    if (poll) {
        std::string_view cmd = "AT+USORF=0,0\r";

        if (mSend(cmd, mTimeout) != cmd.length()) {
            return false;
        }
    }

    AtCmd::ParseResult parsed;
    std::string_view response;
    while (true) {
        response = readLine();
        if (response.length() == 0) {
            return false;
        }
        parsed = parseResponse(response);
        if ((parsed == AtCmd::ParseResult::USORF) || (parsed == AtCmd::ParseResult::UUSORF) ||
            (parsed == AtCmd::ParseResult::UUSORD))
        {
            Trace(ZONE_INFO, "USORF, UUSORF, UUSORD found\r\n");

            break;
        }
    }

    if ((parsed == AtCmd::ParseResult::USORF) && (waitForErrorOrOk() == false)) {
        return false;
    }
    std::vector<std::string_view> responseParts = splitDataString(response);

    if (responseParts.size() != 3) {
        return false;
    }

    size_t availableBytes = std::stoi(std::string(responseParts[2].data(), responseParts[2].length()));

    if (availableBytes == 0) {
        return false;
    }

    Trace(ZONE_INFO, "Read %s bytes \r\n", std::string(responseParts[2].data(), responseParts[2].length()).c_str());

    std::string readcmd = "AT+USORF=0," + std::to_string(availableBytes) + "\r";
    if (mSend(readcmd, mTimeout) != readcmd.length()) {
        return false;
    }

    Trace(ZONE_INFO, "Send cmd %s\r\n", readcmd.c_str());

    while (true) {
        std::string_view cmdString = readAtCmd();
        if (cmdString.length() == 0) {
            return false;
        }
        Trace(ZONE_INFO, "Read cmd %s\r\n", std::string(cmdString.data(), cmdString.length()).c_str());
        if (parseResponse(cmdString) == AtCmd::ParseResult::USORF) {
            break;
        }
    }
    Trace(ZONE_INFO, "USORF found\r\n");

    auto socketstring = readUntilComma();
    if (socketstring.length() == 0) {
        return false;
    }
    auto ipstring = readUntilComma();
    if (ipstring.length() == 0) {
        return false;
    }
    auto portstring = readUntilComma();
    if (portstring.length() == 0) {
        return false;
    }
    auto datalengthstring = readUntilComma();
    if (datalengthstring.length() == 0) {
        return false;
    }

    size_t datalength = std::stoi(std::string(datalengthstring.data(), datalengthstring.length()));

    Trace(ZONE_INFO, "DataLength %d\r\n", datalength);

    auto datastring = readMultipleBytes(datalength);
    if (datastring.length() < datalength) {
        return false;
    }

    auto first = datastring.find_first_of('"') + 1;
    auto last = datastring.find_last_of('"');

    receivedData = std::string(datastring.data() + first, last - first);
    Trace(ZONE_VERBOSE, "Got data: %s \r\n", receivedData.c_str());
    return waitForErrorOrOk();
}

std::string_view ReceiveAtCmd::readAtCmd(void) const
{
    uint8_t data;
    size_t lineEndIndex = 0;
    while (mReceive(data, mTimeout) && lineEndIndex < ReceiveBuffer.size()) {
        ReceiveBuffer[lineEndIndex++] = static_cast<char>(data);

        const bool terminationFound = (data == ':');

        if (terminationFound) {
            break;
        }
    }
    return std::string_view(ReceiveBuffer.data(), lineEndIndex);
}

std::string_view ReceiveAtCmd::readUntilComma(void) const
{
    uint8_t data;
    size_t lineEndIndex = 0;
    while (mReceive(data, mTimeout) && lineEndIndex < ReceiveBuffer.size()) {
        const bool terminationFound = (data == ',');

        if (terminationFound) {
            break;
        }
        const bool invalidCharFound = (data >= 'A' && data <= 'Z') || (data >= 'a' && data <= 'z');

        if (invalidCharFound) {
            return std::string_view(ReceiveBuffer.data(), 0);
        }
        ReceiveBuffer[lineEndIndex++] = static_cast<char>(data);
        Trace(ZONE_INFO, "Read_comma: %c \r\n", data);
    }
    return std::string_view(ReceiveBuffer.data(), lineEndIndex);
}

std::string_view ReceiveAtCmd::readMultipleBytes(size_t numberOfBytes) const
{
    uint8_t data;
    size_t lineEndIndex = 0;
    while (mReceive(data, mTimeout) && lineEndIndex < ReceiveBuffer.size()) {
        ReceiveBuffer[lineEndIndex++] = static_cast<char>(data);
        Trace(ZONE_INFO, "Read_multiple: 0x%02X \r\n", data);

        const bool terminationFound = (data == '\r') || (data == '\n') || (data == 0);

        if (terminationFound && (lineEndIndex >= numberOfBytes)) {
            break;
        }
    }
    Trace(ZONE_INFO, "Read_multiple finish %d : %d \r\n", numberOfBytes, lineEndIndex);

    return std::string_view(ReceiveBuffer.data(), lineEndIndex);
}

/* Parses the response to an AT+USORF command,
 * the response is composed of 5 pieces, separated by delimeter ",",
 * +USORF: SOCKET_IDX,"IP_ADDRESS",PORT,DATA_LEN,"DATA".
 * It is also possible that the response is only 2 pieces
 * +USORF: SOCKET_IDX,DATA_LEN
 * This indicates only how much data is in the buffer*/
const std::vector<std::string_view> ReceiveAtCmd::splitDataString(std::string_view input) const
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
            auto lenghtOfLastString = input.length() - startindex;
            if ((input.data()[input.length() - 1] == '\r') || (input.data()[input.length() - 1] == '\n')) {
                lenghtOfLastString--;
            }
            strings.emplace_back(input.data() + startindex, lenghtOfLastString);
            return strings;
        }
        strings.emplace_back(input.data() + startindex, endindex - startindex);
    }
    endindex++;
    auto lenghtOfLastString = input.length() - endindex;
    if ((input.data()[input.length() - 1] == '\r') || (input.data()[input.length() - 1] == '\n')) {
        lenghtOfLastString--;
    }
    strings.emplace_back(input.data() + endindex, lenghtOfLastString);

    return strings;
}
