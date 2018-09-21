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
#include <string>
#include <string_view>
#include <functional>
#include <chrono>
#include <vector>
#include <memory>
#include "Semaphore.h"
#include "Mutex.h"

#define AT_CMD_IP "95.143.172.237"
#define AT_CMD_PORT "64487"

//#define AT_CMD_USOST "AT+USOST=0,\"" AT_CMD_IP "\"," AT_CMD_PORT ","
//#define AT_CMD_USOCO "AT+USOCO=0,\"" AT_CMD_IP "\"," AT_CMD_PORT "\r"

namespace app
{
class ATParser;
class ATCmd;
class ATCmdOK;
class ATCmdERROR;

struct AT {
    using ReceiveFunction = std::function<size_t(uint8_t*, const size_t, std::chrono::milliseconds)>;
    using SendFunction = std::function<size_t(std::string_view, std::chrono::milliseconds)>;

    enum class Return_t
    {
        FINISHED,
        WAITING,
        ERROR,
        TRY_AGAIN
    };

    const std::string_view mName;
    const std::string_view mResponse;
    ATParser& mParser;

protected:

    AT(const std::string_view name,
       const std::string_view response,
       ATParser&              parser) :
        mName(name), mResponse(response), mParser(parser){};
    virtual ~AT(void){};

    virtual void okReceived(void);
    virtual void errorReceived(void);
    virtual Return_t onResponseMatch(void);

    friend class ATParser;
    friend class ATCmdERROR;
    friend class ATCmdOK;
};

class ATCmd :
    public AT, public std::enable_shared_from_this<ATCmd>
{
protected:
    bool mCommandSuccess = false;
    std::string_view mRequest;
    os::Semaphore mSendDone;

    virtual void okReceived(void) override;
    virtual void errorReceived(void) override;

public:

    ATCmd(std::string_view name, std::string_view request, std::string_view response, ATParser& parser) :
        AT(name, response, parser),
        mRequest(request), mSendDone() {};
    virtual ~ATCmd(void){};

    Return_t send(SendFunction& sendFunction, std::chrono::milliseconds timeout);

    bool wasExecutionSuccessful(void) const;
};

class ATCmdUSOST final :
    public ATCmd
{
    size_t mSocket = 0;
    std::string mIp = AT_CMD_IP;
    std::string mPort = AT_CMD_PORT;
    std::string_view mData;
    SendFunction& mSendFunction;
    bool mWaitingForPrompt = false;
    virtual Return_t onResponseMatch(void) override;

public:
    ATCmdUSOST(SendFunction & send, ATParser & parser) :
        ATCmd("AT+USOST", "", "@", parser), mSendFunction(send){}

    Return_t send(std::string_view data, std::chrono::milliseconds timeout);
    Return_t send(std::string_view data, std::string_view ip, std::string_view port, std::chrono::milliseconds timeout);
};

class ATCmdUSORF final :
    public ATCmd
{
    size_t mSocket = 0;
    std::string mIp = AT_CMD_IP;
    std::string mPort = AT_CMD_PORT;
    std::string mData;
    SendFunction& mSendFunction;
    const std::function<void(size_t, size_t)>& mUrcReceivedCallback;
    virtual Return_t onResponseMatch(void) override;

public:
    ATCmdUSORF(SendFunction & send, ATParser & parser, const std::function<void(size_t, size_t)> &callback) :
        ATCmd("AT+USORF", "", "+USORF:", parser), mSendFunction(send), mUrcReceivedCallback(callback){}

    Return_t send(size_t bytesToRead, std::chrono::milliseconds timeout);

    const std::string& getData(void) const
    {
        return mData;
    }
};

class ATCmdUPSND final :
    public ATCmd
{
    size_t mSocket = 0;
    size_t mParameter = 0;
    SendFunction& mSendFunction;
    std::string mData;
    virtual Return_t onResponseMatch(void) override;

public:
    ATCmdUPSND(SendFunction & send, ATParser & parser) :
        ATCmd("AT+UPSND", "", "+UPSND:", parser), mSendFunction(send){}

    Return_t send(const size_t socket, const size_t parameter, const std::chrono::milliseconds timeout);

    const std::string& getData(void) const
    {
        return mData;
    }

    size_t getParameter(void) const
    {
        return mParameter;
    }

    size_t getSocket(void) const
    {
        return mSocket;
    }
};

class ATCmdURC final :
    public AT
{
    virtual Return_t onResponseMatch(void) override;
    const std::function<void(size_t, size_t)>& mUrcReceivedCallback;

public:
    ATCmdURC(const std::string_view name, const std::string_view response, ATParser & parser,
             const std::function<void(size_t, size_t)> &callback) :
        AT(name, response, parser), mUrcReceivedCallback(callback) {}
};

class ATCmdOK final :
    public AT
{
    Return_t onResponseMatch(void) override;

public:
    ATCmdOK(ATParser & parser) :
        AT("CMD_OK", "OK\r", parser) {};
    virtual ~ATCmdOK(void){};
    friend class ATParser;
};

class ATCmdERROR final :
    public AT
{
    Return_t onResponseMatch(void) override;

public:
    ATCmdERROR(ATParser & parser) :
        AT("CMD_ERROR", "ERROR\r", parser) {};
    virtual ~ATCmdERROR(void){};
    friend class ATParser;
};

struct ATParser {
    static constexpr const size_t BUFFERSIZE = 512;

    ATParser(const AT::ReceiveFunction& receive) :
        mReceive(receive) {}

    void reset(void);

    bool parse(std::chrono::milliseconds timeout = std::chrono::milliseconds(10000));
    void registerAtCommand(std::shared_ptr<AT> cmd);
    std::string_view getLineFromInput(std::chrono::milliseconds timeout = std::chrono::milliseconds(500)) const;
    std::string_view getInputUntilComma(std::chrono::milliseconds timeout = std::chrono::milliseconds(500),
                                        char*                     termination = nullptr) const;
    std::string_view getBytesFromInput(size_t                    numberOfBytes,
                                       std::chrono::milliseconds timeout = std::chrono::milliseconds(500)) const;

protected:
    static std::array<char, BUFFERSIZE> ReceiveBuffer;

    const AT::ReceiveFunction& mReceive;
    std::vector<std::shared_ptr<AT> > mRegisteredATCommands;
    std::shared_ptr<AT> mWaitingCmd;
    os::Mutex mSendingMutex;

    friend class ATCmdOK;
    friend class ATCmdERROR;
    friend class ATCmd;
};
}

#endif /* SOURCES_PMD_AT_PARSER_H_ */
