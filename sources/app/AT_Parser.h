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
#include "Semaphore.h"
#include "Mutex.h"
#include "Endpoint.h"

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

    ATParser* mParser;

protected:

    AT(const std::string_view name,
       const std::string_view response) :
        mName(name), mResponse(response)
    {
        mParser = nullptr;
    };
    virtual ~AT(void){};

    virtual void okReceived(void);
    virtual void errorReceived(void);
    virtual Return_t onResponseMatch(void);

    friend class ATParser;
    friend class ATCmdERROR;
    friend class ATCmdOK;
};

class ATCmd :
    public AT
{
protected:
    bool mCommandSuccess = false;
    std::string_view mRequest;
    os::Semaphore mSendDone;

    virtual void okReceived(void) override;
    virtual void errorReceived(void) override;

public:

    ATCmd(std::string_view name, std::string_view request, std::string_view response) :
        AT(name, response),
        mRequest(request), mSendDone() {};
    virtual ~ATCmd(void){};

    Return_t send(SendFunction& sendFunction, std::chrono::milliseconds timeout);

    bool wasExecutionSuccessful(void) const;
};

class ATCmdUSOST final :
    public ATCmd
{
    std::string_view mData;
    SendFunction& mSendFunction;
    virtual Return_t onResponseMatch(void) override;

public:
    ATCmdUSOST(SendFunction & send) :
        ATCmd("AT+USOST", "", "@"), mSendFunction(send){}

    Return_t send(const size_t              socket,
                  std::string_view          ip,
                  std::string_view          port,
                  std::string_view          data,
                  std::chrono::milliseconds timeout);
};

class ATCmdUSOWR final :
    public ATCmd
{
    std::string_view mData;
    SendFunction& mSendFunction;
    virtual Return_t onResponseMatch(void) override;

public:
    ATCmdUSOWR(SendFunction & send) :
        ATCmd("AT+USOWR", "", "@"), mSendFunction(send){}

    Return_t send(const size_t              socket,
                  std::string_view          data,
                  std::chrono::milliseconds timeout);
};

class ATCmdUSORF final :
    public ATCmd
{
    size_t mSocket = 0;
    std::string mIp;
    std::string mPort;
    std::string mData;
    SendFunction& mSendFunction;
    const std::function<void(size_t, size_t)>& mUrcReceivedCallback;
    virtual Return_t onResponseMatch(void) override;

public:
    ATCmdUSORF(SendFunction & send, const std::function<void(size_t, size_t)> &callback) :
        ATCmd("AT+USORF", "", "+USORF:"), mSendFunction(send), mUrcReceivedCallback(callback){}

    Return_t send(size_t socket, size_t bytesToRead, std::chrono::milliseconds timeout);

    const std::string& getData(void) const
    {
        return mData;
    }
};

class ATCmdUSORD final :
    public ATCmd
{
    size_t mSocket = 0;
    std::string mData;
    SendFunction& mSendFunction;
    const std::function<void(size_t, size_t)>& mUrcReceivedCallback;
    virtual Return_t onResponseMatch(void) override;

public:
    ATCmdUSORD(SendFunction & send, const std::function<void(size_t, size_t)> &callback) :
        ATCmd("AT+USORD", "", "+USORD:"), mSendFunction(send), mUrcReceivedCallback(callback){}

    Return_t send(size_t socket, size_t bytesToRead, std::chrono::milliseconds timeout);

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
    ATCmdUPSND(SendFunction & send) :
        ATCmd("AT+UPSND", "", "+UPSND:"), mSendFunction(send){}

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

class ATCmdUSOCR final :
    public ATCmd
{
    size_t mSocket = 0;
    SendFunction& mSendFunction;
    virtual Return_t onResponseMatch(void) override;

public:
    ATCmdUSOCR(SendFunction & send) :
        ATCmd("AT+USOCR", "", "+USOCR:"), mSendFunction(send){}

    Return_t send(const size_t protocol, const std::chrono::milliseconds timeout);

    size_t getSocket(void) const
    {
        return mSocket;
    }
};

class ATCmdUSOCO final :
    public ATCmd
{
    SendFunction& mSendFunction;

public:
    ATCmdUSOCO(SendFunction & send) :
        ATCmd("AT+USOCO", "", ""), mSendFunction(send) {}

    Return_t send(size_t                    socket,
                  std::string_view          ip,
                  std::string_view          port,
                  std::chrono::milliseconds timeout);
};

class ATCmdURC final :
    public AT
{
    virtual Return_t onResponseMatch(void) override;
    const std::function<void(size_t, size_t)>& mUrcReceivedCallback;

public:
    ATCmdURC(const std::string_view name, const std::string_view response,
             const std::function<void(size_t, size_t)> &callback) :
        AT(name, response), mUrcReceivedCallback(callback) {}
};

class ATCmdOK final :
    public AT
{
    Return_t onResponseMatch(void) override;

public:
    ATCmdOK(void) :
        AT("CMD_OK", "OK\r") {};
    virtual ~ATCmdOK(void){};
    friend class ATParser;
};

class ATCmdERROR final :
    public AT
{
    Return_t onResponseMatch(void) override;

public:
    ATCmdERROR(void) :
        AT("CMD_ERROR", "ERROR\r") {};
    virtual ~ATCmdERROR(void){};
    friend class ATParser;
};

struct ATParser {
    static constexpr const size_t BUFFERSIZE = 512;
    static constexpr const std::chrono::milliseconds defaultTimeout = std::chrono::milliseconds(300);
    static constexpr const std::chrono::milliseconds defaultParseTimeout = std::chrono::milliseconds(30000);

    ATParser(const AT::ReceiveFunction& receive) :
        mReceive(receive), mWaitingCmd(nullptr), mWaitingCmdMutex() {}

    void reset(void);
    void triggerMatch(AT* match);
    bool parse(std::chrono::milliseconds timeout = defaultParseTimeout);
    void registerAtCommand(AT& cmd);
    std::string_view getLineFromInput(std::chrono::milliseconds timeout = defaultTimeout) const;
    std::string_view getInputUntilComma(char*                     termination = nullptr,
                                        std::chrono::milliseconds timeout = defaultTimeout) const;
    std::string_view getBytesFromInput(size_t                    numberOfBytes,
                                       std::chrono::milliseconds timeout = defaultTimeout) const;

protected:
    static std::array<char, BUFFERSIZE> ReceiveBuffer;

    const AT::ReceiveFunction& mReceive;
    std::vector<AT*> mRegisteredATCommands;
    AT* mWaitingCmd;
    os::Mutex mWaitingCmdMutex;

    friend class ATCmdOK;
    friend class ATCmdERROR;
    friend class ATCmd;
};
}

#endif /* SOURCES_PMD_AT_PARSER_H_ */
