// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include <string_view>
#include <array>
#include "AT_Parser.h"
#include "os_Queue.h"
#include "os_StreamBuffer.h"

namespace app
{
class ModemDriver;

class Socket
{
protected:
    static constexpr const size_t BUFFERSIZE = 512;
    static constexpr const std::chrono::milliseconds KEEP_ALIVE_PAUSE = std::chrono::seconds(10);
    static constexpr const char* KEEP_ALIVE_MSG = "\r";

    os::StreamBuffer<char, BUFFERSIZE> mSendBuffer;
    os::StreamBuffer<char, BUFFERSIZE> mReceiveBuffer;
    std::array<char, BUFFERSIZE> mTemporaryBuffer;

    std::function<void(std::string_view)> mReceiveCallback;
    os::Queue<size_t, 1> mNumberOfBytesForReceive;

    virtual void sendData(void) = 0;
    virtual void receiveData(size_t) = 0;
    virtual bool create() = 0;
    virtual bool open(void) = 0;
    virtual void checkIfDataAvailable(void) = 0;

    bool create(size_t magicSocket);
    void reset(void);
    void checkAndReceiveData(void);
    void checkAndSendData(void);
    void storeReceivedData(const std::string_view);
    size_t loadTemporaryBuffer(void);

    ATCmdUSOCR mATCmdUSOCR;
    ATCmdUSOCO mATCmdUSOCO;
    ATCmdUSOSO mATCmdUSOSO;
    ATCmdUSOCTL mATCmdUSOCTL;
    size_t mSocket;
    size_t mTimeOfLastSend;
    size_t mTimeOfLastReceive;

    bool isOpen = false;
    bool isCreated = false;

    const std::function<void(void)> mHandleError;

public:
    enum class Protocol { UDP, TCP, DNS };

    Socket(const Protocol,
           ATParser&                        parser,
           AT::SendFunction&                send,
           const std::string_view           ip,
           const std::string_view           port,
           const std::function<void(void)>& errorCallback);

    Socket(const Socket&) = delete;
    Socket(Socket&&) = delete;
    Socket& operator=(const Socket&) = delete;
    Socket& operator=(Socket&&) = delete;

    virtual ~Socket(void);

    const Protocol mProtocol;
    const std::string_view mIP;
    const std::string_view mPort;

    size_t send(std::string_view, const uint32_t ticksToWait = portMAX_DELAY);
    size_t receive(uint8_t*, size_t, uint32_t ticksToWait = portMAX_DELAY);

    size_t bytesAvailable(void) const;
    size_t getTimeOfLastSend(void) const;

    void registerReceiveCallback(std::function<void(std::string_view)> );
    void unregisterReceiveCallback(void);

    friend ModemDriver;
};

class TcpSocket :
    public Socket
{
    virtual void sendData(void) override;
    virtual void receiveData(size_t) override;
    virtual bool create(void) override;
    virtual bool open(void) override;
    virtual void checkIfDataAvailable(void) override;

    ATCmdUSOWR mATCmdUSOWR;
    ATCmdUSORD mATCmdUSORD;

public:
    TcpSocket(ATParser& parser,
              AT::SendFunction& send,
              const std::string_view ip,
              const std::string_view port,
              const std::function<void(size_t, size_t)>& callback,
              const std::function<void(void)>& errorCallback);

    TcpSocket(const TcpSocket&) = delete;
    TcpSocket(TcpSocket&&) = delete;
    TcpSocket& operator=(const TcpSocket&) = delete;
    TcpSocket& operator=(TcpSocket&&) = delete;

    virtual ~TcpSocket(void);

    friend ModemDriver;
};

class UdpSocket :
    public Socket
{
protected:
    virtual void sendData(void) override;
    virtual void receiveData(size_t) override;
    virtual bool create(void) override;
    virtual bool open(void) override;
    virtual void checkIfDataAvailable(void) override;

    ATCmdUSOST mATCmdUSOST;
    ATCmdUSORF mATCmdUSORF;

public:
    UdpSocket(ATParser& parser,
              AT::SendFunction& send,
              std::string_view ip,
              std::string_view port,
              const std::function<void(size_t, size_t)>& callback,
              const std::function<void(void)>& errorCallback);

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket(UdpSocket&&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;
    UdpSocket& operator=(UdpSocket&&) = delete;

    virtual ~UdpSocket(void);

    friend ModemDriver;
};

class DnsSocket :
    public UdpSocket
{
    virtual void sendData(void) override;
    virtual void receiveData(size_t) override;
    virtual bool open(void) override;

    bool queryDnsServerIP(void);
    std::string_view getDnsServerIP(void);

    ATCmdUPSND mATCmdUPSND;

public:
    DnsSocket(ATParser& parser,
              AT::SendFunction& send,
              const std::function<void(size_t, size_t)>& callback,
              const std::function<void(void)>& errorCallback);

    DnsSocket(const DnsSocket&) = delete;
    DnsSocket(DnsSocket&&) = delete;
    DnsSocket& operator=(const DnsSocket&) = delete;
    DnsSocket& operator=(DnsSocket&&) = delete;

    virtual ~DnsSocket(void);

    friend ModemDriver;
};
}
