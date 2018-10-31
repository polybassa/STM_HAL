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

#include "Socket.h"
#include "trace.h"
#include "binascii.h"
#include "os_Task.h"
#include <algorithm>

using app::DnsSocket;
using app::Socket;
using app::TcpSocket;
using app::UdpSocket;

static const int __attribute__((unused)) g_DebugZones = 0; //ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

Socket::Socket(const Protocol                   protocol,
               ATParser&                        parser,
               AT::SendFunction&                send,
               const std::string_view           ip,
               const std::string_view           port,
               const std::function<void(void)>& errorCallback) :
    mNumberOfBytesForReceive(),
    mATCmdUSOCR(send),
    mATCmdUSOCO(send),
    mATCmdUSOSO(send),
    mSocket(0),
    mTimeOfLastSend(os::Task::getTickCount()),
    mHandleError(errorCallback),
    mProtocol(protocol),
    mIP(ip),
    mPort(port)
{
    parser.registerAtCommand(&mATCmdUSOCR);
    parser.registerAtCommand(&mATCmdUSOCO);
    parser.registerAtCommand(&mATCmdUSOSO);
}

Socket::~Socket(void){}

void Socket::reset(void)
{
    mReceiveBuffer.reset();
    mNumberOfBytesForReceive.reset();
    isOpen = false;
    isCreated = false;
}

void Socket::checkAndReceiveData(void)
{
    size_t bytes = 0;

    if (isOpen && mNumberOfBytesForReceive.receive(bytes, std::chrono::milliseconds(10))) {
        Trace(ZONE_VERBOSE, "receive\r\n");

        this->receiveData(bytes);
    }
}

void Socket::checkAndSendData(void)
{
    if (isOpen && mSendBuffer.bytesAvailable()) {
        Trace(ZONE_VERBOSE, "send\r\n");

        this->sendData();
    }

    if ((mProtocol == Protocol::UDP) && (os::Task::getTickCount() - mTimeOfLastSend >= KEEP_ALIVE_PAUSE.count())) {
        send(KEEP_ALIVE_MSG, std::chrono::milliseconds(100).count());
    }
}

void Socket::storeReceivedData(const std::string_view data)
{
    Trace(ZONE_INFO, "Data will be stored %d\r\n", data.length());

    if (mReceiveCallback) {
        mReceiveCallback(data);
    } else {
        mReceiveBuffer.send(data.data(), data.length(), 1000);
    }
    Trace(ZONE_INFO, "Data stored\r\n");
}

size_t Socket::loadTemporaryBuffer(void)
{
    const size_t bytes = std::min(mTemporaryBuffer.size(), mSendBuffer.bytesAvailable());

    Trace(ZONE_VERBOSE, "Send %d \r\n", bytes);

    const size_t receivedLength = mSendBuffer.receive(mTemporaryBuffer.data(), mTemporaryBuffer.size(), 1000);

    if (receivedLength != bytes) {
        Trace(ZONE_ERROR, "Internal buffer didn't contain exact amount of bytes\r\n");
        return 0;
    }
    return receivedLength;
}

size_t Socket::send(std::string_view message, const uint32_t ticksToWait)
{
    return mSendBuffer.send(message.data(), message.length(), ticksToWait);
}

size_t Socket::receive(uint8_t* message, size_t length, uint32_t ticksToWait)
{
    return mReceiveBuffer.receive(reinterpret_cast<char*>(message), length, ticksToWait);
}

size_t Socket::bytesAvailable(void) const
{
    return mReceiveBuffer.bytesAvailable();
}

size_t Socket::getTimeOfLastSend(void) const
{
    return mTimeOfLastSend;
}

void Socket::registerReceiveCallback(std::function<void(std::string_view)> f)
{
    mReceiveCallback = f;
}
void Socket::unregisterReceiveCallback(void)
{
    mReceiveCallback = nullptr;
}

TcpSocket::TcpSocket(ATParser& parser,
                     AT::SendFunction& send,
                     const std::string_view ip,
                     const std::string_view port,
                     const std::function<void(size_t, size_t)>& callback,
                     const std::function<void(void)>& errorCallback) :
    Socket(Protocol::TCP, parser, send, ip, port, errorCallback),
    mATCmdUSOWR(send),
    mATCmdUSORD(send, callback)
{
    parser.registerAtCommand(&mATCmdUSOWR);
    parser.registerAtCommand(&mATCmdUSORD);
}

TcpSocket::~TcpSocket(){}

void TcpSocket::sendData(void)
{
    const size_t receivedLength = loadTemporaryBuffer();
    if (!receivedLength) {
        return;
    }

    if (mATCmdUSOWR.send(mSocket,
                         std::string_view(mTemporaryBuffer.data(),
                                          receivedLength),
                         std::chrono::milliseconds(5000)) == AT::Return_t::ERROR)
    {
        mHandleError();
    }
    mTimeOfLastSend = os::Task::getTickCount();
    mATCmdUSORD.send(mSocket, 0, std::chrono::milliseconds(1000));
}

void TcpSocket::receiveData(size_t bytes)
{
    if (bytes == 0) {
        return;
    }
    Trace(ZONE_INFO, "Start receive %d\r\n", bytes);
    if (mATCmdUSORD.send(mSocket, bytes, std::chrono::milliseconds(1000)) == AT::Return_t::FINISHED) {
        storeReceivedData(mATCmdUSORD.getData());
    } else {
        mHandleError();
    }
}

bool TcpSocket::create()
{
    if (mATCmdUSOCR.send(6, std::chrono::seconds(2)) != AT::Return_t::FINISHED) {
        Trace(ZONE_VERBOSE, "Socket create failed \r\n");

        return false;
    }
    mSocket = mATCmdUSOCR.getSocket();
    Trace(ZONE_VERBOSE, "Socket %d: created \r\n", mSocket);
    isCreated = true;
    isOpen = false;
    return true;
}

bool TcpSocket::open(void)
{
    if (mATCmdUSOCO.send(mSocket, mIP, mPort, std::chrono::seconds(2)) != AT::Return_t::FINISHED) {
        return false;
    }
    isOpen = true;
    Trace(ZONE_VERBOSE, "Socket %d: opened \r\n", mSocket);

    if (mATCmdUSOSO.send(mSocket, 6, 1, 1, std::chrono::seconds(2)) != AT::Return_t::FINISHED) {
        return false;
    }

    if (mATCmdUSOSO.send(mSocket, 6, 2, 10000, std::chrono::seconds(2)) != AT::Return_t::FINISHED) {
        return false;
    }
    Trace(ZONE_VERBOSE, "Socket %d: options set \r\n", mSocket);

    return true;
}

UdpSocket::UdpSocket(ATParser& parser,
                     AT::SendFunction& send,
                     std::string_view ip,
                     std::string_view port,
                     const std::function<void(size_t, size_t)>& callback,
                     const std::function<void(void)>& errorCallback) :
    Socket(Protocol::UDP, parser, send, ip, port, errorCallback),
    mATCmdUSOST(send),
    mATCmdUSORF(send, callback)
{
    parser.registerAtCommand(&mATCmdUSOST);
    parser.registerAtCommand(&mATCmdUSORF);
}

UdpSocket::~UdpSocket(void){}

void UdpSocket::sendData(void)
{
    const size_t receivedLength = loadTemporaryBuffer();
    if (!receivedLength) {
        return;
    }

    if (mATCmdUSOST.send(mSocket, mIP, mPort,
                         std::string_view(mTemporaryBuffer.data(),
                                          receivedLength), std::chrono::milliseconds(5000)) == AT::Return_t::ERROR)
    {
        mHandleError();
    }
    mTimeOfLastSend = os::Task::getTickCount();
    mATCmdUSORF.send(mSocket, 0, std::chrono::milliseconds(1000));
}

void UdpSocket::receiveData(size_t bytes)
{
    if (bytes == 0) {
        return;
    }
    Trace(ZONE_INFO, "S%d: receive %d\r\n", mSocket, bytes);

    if (mATCmdUSORF.send(mSocket, bytes, std::chrono::milliseconds(1000)) == AT::Return_t::FINISHED) {
        storeReceivedData(mATCmdUSORF.getData());
    } else {
        mHandleError();
    }
}

bool UdpSocket::create()
{
    if (mATCmdUSOCR.send(17, std::chrono::seconds(2)) != AT::Return_t::FINISHED) {
        Trace(ZONE_VERBOSE, "Socket create failed \r\n");
        return false;
    }
    mSocket = mATCmdUSOCR.getSocket();
    Trace(ZONE_VERBOSE, "Socket %d: created \r\n", mSocket);

    isCreated = true;
    isOpen = false;
    return true;
}

bool UdpSocket::open(void)
{
    if (mATCmdUSOCO.send(mSocket, mIP, mPort, std::chrono::seconds(2)) != AT::Return_t::FINISHED) {
        return false;
    }
    isOpen = true;
    return true;
}

DnsSocket::DnsSocket(ATParser& parser,
                     AT::SendFunction& send,
                     const std::function<void(size_t, size_t)>& callback,
                     const std::function<void(void)>& errorCallback) :
    UdpSocket(parser, send, "", "", callback, errorCallback),
    mATCmdUPSND(send)
{
    parser.registerAtCommand(&mATCmdUPSND);
}

DnsSocket::~DnsSocket(void){}

void DnsSocket::sendData(void)
{
    //TODO: REFACTOR THIS FUNCTION

    static constexpr const size_t MAX_PAYLOAD_LENGTH = 24;
    static short counter = 900;

    counter++;
    counter %= 999;
    std::string counterStr = std::to_string(counter);

    std::string tmpPayloadStr(std::max(mSendBuffer.bytesAvailable(),
                                       MAX_PAYLOAD_LENGTH), '\x00');
    mSendBuffer.receive(tmpPayloadStr.data(), tmpPayloadStr.length(), 1000);

    std::string hexPayloadStr;
    hexlify(hexPayloadStr, tmpPayloadStr);

    std::string tmpSendStr(
                           "\x00\x00\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00\x33\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x63\x63\x63\x02\x74\x31\x05\x7a\x69\x6e\x6f\x6f\x02\x69\x74\x00\x00\x1c\x00\x01",
                           81);

    tmpSendStr.replace(13, hexPayloadStr.length(), hexPayloadStr);
    tmpSendStr.replace(64 - counterStr.length(), counterStr.length(), counterStr);

    auto ret = mATCmdUSOST.send(mSocket, mATCmdUPSND.getData(), "53", tmpSendStr, std::chrono::milliseconds(1000));

    if (ret == AT::Return_t::ERROR) {
        mHandleError();
    } else if (ret == AT::Return_t::FINISHED) {
        mTimeOfLastSend = os::Task::getTickCount();
    }
    mATCmdUSORF.send(0, 0, std::chrono::milliseconds(1000));
}

void DnsSocket::receiveData(size_t bytes)
{
    //TODO: REFACTOR THIS FUNCTION

    Trace(ZONE_INFO, "Start receive over dns %d\r\n", bytes);
    if (bytes == 0) {
        return;
    }
    auto ret = mATCmdUSORF.send(0, bytes, std::chrono::milliseconds(1000));
    if (ret == AT::Return_t::FINISHED) {
        const auto& data = mATCmdUSORF.getData();

        if (data.length() < 220) {
            Trace(ZONE_VERBOSE, "DNS PKT to short\r\n,");
            return;
        }

        constexpr const auto FRAMEINDICES = {94, 122, 150, 178};
        static constexpr const size_t FRAMELENGTH = 14;
        std::string rawdata("\x00", FRAMELENGTH * FRAMEINDICES.size());

        for (const auto idx : FRAMEINDICES) {
            const auto rawdata1 = data.substr(idx, FRAMELENGTH);
            const auto rawdata1Idx = static_cast<size_t>(data[idx + FRAMELENGTH]) - 1 % 4;
            rawdata.replace(rawdata1Idx * FRAMELENGTH, FRAMELENGTH, rawdata1);
        }

        storeReceivedData(rawdata);
    } else {
        mHandleError();
    }
}

bool DnsSocket::open()
{
    if (!UdpSocket::open()) {
        return false;
    }
    if (!queryDnsServerIP()) {
        isOpen = false;
        return false;
    }
    return true;
}

bool DnsSocket::queryDnsServerIP(void)
{
    const auto ret = mATCmdUPSND.send(mSocket, 1, std::chrono::milliseconds(1000));

    if (ret == AT::Return_t::ERROR) {
        mHandleError();
    }
    return ret == AT::Return_t::FINISHED;
}

std::string_view DnsSocket::getDnsServerIP(void)
{
    return mATCmdUPSND.getData();
}
