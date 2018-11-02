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

#ifndef SOURCES_PMD_SOCKET_H_
#define SOURCES_PMD_SOCKET_H_

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
    static constexpr const std::chrono::milliseconds KEEP_ALIVE_PAUSE = std::chrono::seconds(3);
    static constexpr const char* KEEP_ALIVE_MSG = "ALIVE";

    os::StreamBuffer<uint8_t, BUFFERSIZE> mSendBuffer;
    os::StreamBuffer<uint8_t, BUFFERSIZE> mReceiveBuffer;
    std::array<char, BUFFERSIZE> mTemporaryBuffer;

    std::function<void(std::string_view)> mReceiveCallback;
    os::Queue<size_t, 1> mNumberOfBytesForReceive;

    virtual void sendData(void) = 0;
    virtual void receiveData(size_t) = 0;
    virtual bool create() = 0;
    virtual bool open(void) = 0;

    bool create(size_t magicSocket);
    void reset(void);
    void checkAndReceiveData(void);
    void checkAndSendData(void);
    void storeReceivedData(const std::string_view);
    size_t loadTemporaryBuffer(void);

    ATCmdUSOCR mATCmdUSOCR;
    ATCmdUSOCO mATCmdUSOCO;
    ATCmdUSOSO mATCmdUSOSO;
    size_t mSocket;
    size_t mTimeOfLastSend;
    bool isOpen = false;
    bool isCreated = false;

    const std::function<void(void)>& mHandleError;

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

#endif /* SOURCES_PMD_SOCKET_H_ */
