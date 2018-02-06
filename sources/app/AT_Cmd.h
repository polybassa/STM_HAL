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

#ifndef SOURCES_PMD_AT_CMD_H_
#define SOURCES_PMD_AT_CMD_H_

#include "os_StreamBuffer.h"
#include <string_view>
#include <functional>
#include <chrono>
#include <vector>

#define AT_CMD_IP "91.7.46.108"
#define AT_CMD_PORT "60017"

#define AT_CMD_USOST "AT+USOST=0,\"" AT_CMD_IP "\"," AT_CMD_PORT ","
#define AT_CMD_USOCO "AT+USOCO=0,\"" AT_CMD_IP "\"," AT_CMD_PORT "\r"

namespace app
{
struct AtCmd {
    using ReceiveFunction = std::function<size_t(uint8_t*, const size_t, std::chrono::milliseconds)>;
    using SendFunction = std::function<size_t(std::string_view, std::chrono::milliseconds)>;

    AtCmd(SendFunction&             send,
          ReceiveFunction&          receive,
          std::chrono::milliseconds timeout = std::chrono::milliseconds(6000)) :
        mSend(send), mReceive(receive), mTimeout(timeout){}

protected:
    static constexpr const size_t BUFFERSIZE = 512;
    const SendFunction& mSend;
    const ReceiveFunction& mReceive;
    std::chrono::milliseconds mTimeout;

    static std::array<char, BUFFERSIZE> ReceiveBuffer;

    enum class ParseResult
    {
        USORF,
        UUSORF,
        UUSORD,
        USOST,
        OK,
        ERROR,
        PROMPT,
        NEWLINE,
        CARRIAGE_RETURN,
        UNKNOWN
    };

    std::string_view readLine(void) const;
    AtCmd::ParseResult parseResponse(std::string_view input) const;
    bool waitForErrorOrOk(void) const;
};

class BasicAtCmd :
    protected AtCmd
{
protected:

    std::string_view mRequest;

public:
    BasicAtCmd(std::string_view          request,
               SendFunction&             send,
               ReceiveFunction&          receive,
               std::chrono::milliseconds timeout = std::chrono::milliseconds(6000)) :
        AtCmd(send, receive, timeout), mRequest(request){}

    bool process(void) const;
};

class SendAtCmd :
    protected AtCmd
{
protected:
    std::string_view readUntilPrompt(void) const;

public:
    using AtCmd::AtCmd;

    bool process(std::string_view) const;
};

class ReceiveAtCmd :
    protected AtCmd
{
    const std::vector<std::string_view> splitDataString(std::string_view input) const;

public:
    using AtCmd::AtCmd;

    bool process(std::string& receivedData, bool poll) const;
    std::string_view readAtCmd(void) const;
    std::string_view readUntilComma(void) const;

    std::string_view readMultipleBytes(size_t numberOfBytes) const;
};
}

#endif /* SOURCES_PMD_AT_CMD_H_ */
