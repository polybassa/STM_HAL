// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include <array>
#include <string_view>
#include <functional>
#include <chrono>
#include "os_Queue.h"
#include "Mutex.h"

namespace app
{
class ATParser;
class ATCmd;
class ATCmdOK;
class ATCmdERROR;

struct AT {
    using ReceiveFunction = std::function<size_t(uint8_t*, const size_t, std::chrono::milliseconds)>;
    using SendFunction = std::function<size_t(std::string_view, std::chrono::milliseconds)>;

    enum class Return_t {
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

    virtual void okReceived(void) = 0;
    virtual void errorReceived(void) = 0;
    virtual Return_t onResponseMatch(void) = 0;

    friend class ATParser;
    friend class ATCmd;
    friend class ATCmdOK;
    friend class ATCmdERROR;
};

struct ATCmd :
    AT {
    ATCmd(const std::string_view name, const std::string_view request, const std::string_view response) :
        AT(name, response),
        mRequest(request), mSendResult() {};
    virtual ~ATCmd(void){};

    Return_t send(SendFunction& sendFunction, const std::chrono::milliseconds timeout);

protected:
    std::string_view mRequest;
    os::Queue<bool, 1> mSendResult;

    virtual void okReceived(void) override;
    virtual void errorReceived(void) override;
    virtual Return_t onResponseMatch(void) override;
};

struct ATCmdCGATT final :
    ATCmd {
    ATCmdCGATT(void) :
        ATCmd("AT+CGATT", "AT+CGATT?\r", "+CGATT:"){}

    bool getResult(void) {return mResult;}
private:

    bool mResult = false;
    virtual Return_t onResponseMatch(void) override;
};

struct ATCmdTX :
    ATCmd {
protected:
    std::array<char, 64> mRequestBuffer;
    std::string_view mData;
    SendFunction& mSendFunction;
    virtual Return_t onResponseMatch(void) override;

    ATCmdTX(const std::string_view name, SendFunction& send) :
        ATCmd(name, "", "@"), mSendFunction(send){}
};

struct ATCmdUSOST final :
    ATCmdTX {
    ATCmdUSOST(SendFunction& send) :
        ATCmdTX("AT+USOST", send){}

    Return_t send(const size_t                    socket,
                  const std::string_view          ip,
                  const std::string_view          port,
                  const std::string_view          data,
                  const std::chrono::milliseconds timeout);
};

struct ATCmdUSOWR final :
    ATCmdTX {
    ATCmdUSOWR(SendFunction& send) :
        ATCmdTX("AT+USOWR", send){}

    Return_t send(const size_t                    socket,
                  const std::string_view          data,
                  const std::chrono::milliseconds timeout);
};

struct ATCmdRXData :
    ATCmd {
    std::string_view getData(void) const;

protected:
    std::array<char, 24> mRequestBuffer;
    std::array<char, 256> mDataBuffer;
    std::string_view mData;
    size_t mSocket = 0;
    SendFunction& mSendFunction;
    const std::function<void(const size_t, const size_t)>& mUrcReceivedCallback;

    AT::Return_t getDataFromParser(const size_t bytesAvailable);

    ATCmdRXData(const std::string_view name,
                const std::string_view response,
                SendFunction& send,
                const std::function<void(size_t, size_t)>& callback) :
        ATCmd(name, "", response),
        mSendFunction(send),
        mUrcReceivedCallback(callback){}
};

struct ATCmdUSORF final :
    ATCmdRXData {
    ATCmdUSORF(SendFunction& send, const std::function<void(size_t, size_t)>& callback) :
        ATCmdRXData("AT+USORF", "+USORF:", send, callback){}

    Return_t send(const size_t socket, size_t bytesToRead, const std::chrono::milliseconds timeout);

private:

    static constexpr const size_t IPLEN = 16;
    static constexpr const size_t PORTLEN = 6;
    std::array<char, IPLEN> mIpBuffer;
    std::string_view mIp;
    std::array<char, PORTLEN> mPortBuffer;
    std::string_view mPort;
    virtual Return_t onResponseMatch(void) override;
};

struct ATCmdUSORD final :
    ATCmdRXData {
    ATCmdUSORD(SendFunction& send, const std::function<void(size_t, size_t)>& callback) :
        ATCmdRXData("AT+USORD", "+USORD:", send, callback){}

    Return_t send(const size_t socket, size_t bytesToRead, const std::chrono::milliseconds timeout);

private:
    virtual Return_t onResponseMatch(void) override;
};

struct ATCmdUPSND final :
    ATCmd {
    ATCmdUPSND(SendFunction& send) :
        ATCmd("AT+UPSND", "", "+UPSND:"), mSendFunction(send){}

    Return_t send(const size_t socket, const size_t parameter, const std::chrono::milliseconds timeout);
    const std::string_view getData(void) const;
    size_t getParameter(void) const;
    size_t getSocket(void) const;
private:
    std::array<char, 16> mRequestBuffer;
    std::array<char, 32> mDataBuffer;
    size_t mSocket = 0;
    size_t mParameter = 0;
    SendFunction& mSendFunction;
    std::string_view mData;
    virtual Return_t onResponseMatch(void) override;
};

struct ATCmdUSOCR final :
    ATCmd {
    ATCmdUSOCR(SendFunction& send) :
        ATCmd("AT+USOCR", "", "+USOCR:"), mSendFunction(send){}

    Return_t send(const size_t protocol, const std::chrono::milliseconds timeout);
    size_t getSocket(void) const;

private:
    std::array<char, 16> mRequestBuffer;
    size_t mSocket = 0;
    SendFunction& mSendFunction;
    virtual Return_t onResponseMatch(void) override;
};

struct ATCmdUSOCO final :
    ATCmd {
    ATCmdUSOCO(SendFunction& send) :
        ATCmd("AT+USOCO", "", ""), mSendFunction(send) {}

    Return_t send(const size_t                    socket,
                  const std::string_view          ip,
                  const std::string_view          port,
                  const std::chrono::milliseconds timeout);
private:
    std::array<char, 40> mRequestBuffer;
    SendFunction& mSendFunction;
};

struct ATCmdUSOSO final :
    ATCmd {
    ATCmdUSOSO(SendFunction& send) :
        ATCmd("AT+USOSO", "", ""), mSendFunction(send) {}

    Return_t send(const size_t                    socket,
                  const size_t                    level,
                  const size_t                    optName,
                  const size_t                    optVal,
                  const std::chrono::milliseconds timeout);
private:
    std::array<char, 64> mRequestBuffer;
    SendFunction& mSendFunction;
};

struct ATCmdUSOCTL final :
    ATCmd {
    ATCmdUSOCTL(SendFunction& send) :
        ATCmd("AT+USOCTL", "", "+USOCTL:"), mSendFunction(send) {}

    Return_t send(const size_t                    socket,
                  const size_t                    paramId,
                  const std::chrono::milliseconds timeout);

    const std::string_view getData(void) const;
    size_t getParamId(void) const;
    size_t getSocket(void) const;
private:
    std::array<char, 64> mRequestBuffer;
    SendFunction& mSendFunction;
    std::array<char, 32> mDataBuffer;
    std::string_view mData;
    size_t mSocket = 0;
    size_t mParamId = 0;
    virtual Return_t onResponseMatch(void) override;
};

struct ATCmdURC final :
    AT {
    ATCmdURC(const std::string_view name, const std::string_view response,
             const std::function<void(const size_t, const size_t)>& callback) :
        AT(name, response), mUrcReceivedCallback(callback) {}
private:
    virtual void okReceived(void) override;
    virtual void errorReceived(void) override;
    virtual Return_t onResponseMatch(void) override;

    const std::function<void(const size_t, const size_t)>& mUrcReceivedCallback;
};

struct ATCmdOK final :
    AT {
    ATCmdOK(void) :
        AT("CMD_OK", "OK\r") {};
    virtual ~ATCmdOK(void){};
private:
    virtual void okReceived(void) override;
    virtual void errorReceived(void) override;
    virtual Return_t onResponseMatch(void) override;
    friend class ATParser;
};

struct ATCmdERROR final :
    AT {
    ATCmdERROR(void) :
        AT("CMD_ERROR", "ERROR\r") {};
    virtual ~ATCmdERROR(void){};
    friend class ATParser;
private:
    virtual void okReceived(void) override;
    virtual void errorReceived(void) override;
    virtual Return_t onResponseMatch(void) override;
};

struct ATParser final {
    static constexpr const size_t BUFFERSIZE = 512;
    static constexpr const size_t MAXATCMDS = 32;
    static constexpr const std::chrono::milliseconds defaultTimeout = std::chrono::milliseconds(300);
    static constexpr const std::chrono::milliseconds defaultParseTimeout = std::chrono::milliseconds(45000);

    static bool isValueTermination(const char c);
    static bool isLineTermination(const char c);

    ATParser(const AT::ReceiveFunction& receive);

    void reset(void);
    void triggerMatch(AT* match);
    bool parse(std::chrono::milliseconds timeout = defaultParseTimeout);
    void registerAtCommand(AT* cmd);
    std::string_view getLineFromInput(std::chrono::milliseconds timeout = defaultTimeout) const;
    std::string_view getInputUntilComma(char* const               termination = nullptr,
                                        std::chrono::milliseconds timeout = defaultTimeout) const;
    std::string_view getBytesFromInput(size_t                    numberOfBytes,
                                       std::chrono::milliseconds timeout = defaultTimeout) const;
    AT::Return_t getSocketFromInput(size_t&                   socket,
                                    char* const               termination = nullptr,
                                    std::chrono::milliseconds timeout = defaultTimeout) const;
    AT::Return_t getNumberFromInput(size_t&                   number,
                                    char* const               termination = nullptr,
                                    std::chrono::milliseconds timeout = defaultTimeout) const;
    AT::Return_t strToNum(size_t&                number,
                          const std::string_view numstring) const;

private:
    static std::array<char, BUFFERSIZE> ReceiveBuffer;

    const AT::ReceiveFunction& mReceive;
    std::array<AT*, MAXATCMDS> mRegisteredATCommands;
    size_t mNumberOfRegisteredATCommands = 0;
    AT* mWaitingCmd;
    os::Mutex mWaitingCmdMutex;

    friend class ATCmdOK;
    friend class ATCmdERROR;
    friend class ATCmd;
};
}
