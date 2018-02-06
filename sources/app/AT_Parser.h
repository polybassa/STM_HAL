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

#ifndef SOURCES_PMD_AT_PARSER_H_
#define SOURCES_PMD_AT_PARSER_H_

#include "os_StreamBuffer.h"
#include <string_view>
#include <functional>
#include <chrono>
#include <vector>
#include "for_each_tuple.h"

namespace app
{
class ATParser;

struct AT {
    using ReceiveFunction = std::function<size_t(uint8_t*, const size_t, std::chrono::milliseconds)>;
    using SendFunction = std::function<size_t(std::string_view, std::chrono::milliseconds)>;

    enum class ReturnType
    {
        FINISHED,
        WAITING,
        ERROR
    };

    const std::string_view mName;
    const std::string_view mResponse;

    bool isCommandFinished(void) const;

    AT(std::string_view name, std::string_view response) :
        mName(name), mResponse(response) {};

protected:

    bool mCommandFinished = false;

    void okReceived(void) const;
    void errorReceived(void) const;
    ReturnType onResponseMatch(AT::ReceiveFunction&) const;

    friend class ATParser;
};

class ATCmd :
    public AT
{
    const std::string_view mRequest;

public:

    ATCmd(std::string_view name, std::string_view request, std::string_view response) :
        AT(name, response),
        mRequest(request) {};
};

class ATCmdOK_ERROR final :
    public AT
{
    std::function<void(void)>& mWaitingCmd;
    ReturnType onResponseMatch(AT::ReceiveFunction&) const;

public:
    ATCmdOK_ERROR(std::string_view name, std::string_view response, std::function<void(void)> &waitingCmdCallback) :
        AT(name, response), mWaitingCmd(waitingCmdCallback) {};

    friend class ATParser;
};

struct ATParser {
    ATParser(AT::SendFunction&         send,
             AT::ReceiveFunction&      receive,
             std::chrono::milliseconds timeout = std::chrono::milliseconds(6000)) :
        mSend(send), mReceive(receive), mTimeout(timeout) {}

    bool parse(void) const;

protected:
    static constexpr const size_t BUFFERSIZE = 512;
    AT::SendFunction& mSend;
    AT::ReceiveFunction& mReceive;
    std::chrono::milliseconds mTimeout;

    static std::array<char, BUFFERSIZE> ReceiveBuffer;

    static std::function<void(void)> placeholder1;
    static std::function<void(void)> placeholder2;

    std::function<void(void)>& mWaitingCmdOk = placeholder1;
    std::function<void(void)>& mWaitingCmdError = placeholder2;

    std::tuple<ATCmd, ATCmd, ATCmd, ATCmdOK_ERROR, ATCmdOK_ERROR> SupportedAtCmds {
        ATCmd("CMD_1", "REQ1", "RESP1"),
        ATCmd("CMD_2", "REQ2", "RESP2"),
        ATCmd("CMD_3", "REQ3", "REsp3"),
        ATCmdOK_ERROR("CMD_OK", "OK\r", mWaitingCmdOk),
        ATCmdOK_ERROR("CMD_ERROR", "ERROR\r", mWaitingCmdError)
    };

    template<typename ... types>
    decltype(auto) getAllResponses(void) const
    {
        std::vector<std::tuple<std::string_view, std::function<AT::ReturnType(AT::ReceiveFunction&)>,
                               std::function<void(void)>, std::function<void(void)> > > possibleResponses;

        for_each(SupportedAtCmds, [&possibleResponses](const auto & x){
                     possibleResponses.push_back(std::make_tuple(
                                                                 x.mResponse,
                                                                 [&x](AT::ReceiveFunction & recv)->AT::ReturnType
                                                                 {
                                                                     return x.onResponseMatch(recv);
                                                                 },
                                                                 [&x] {x.okReceived(); },
                                                                 [&x] {x.errorReceived(); }));
                 });
        return possibleResponses;
    }
};
}

#endif /* SOURCES_PMD_AT_PARSER_H_ */
