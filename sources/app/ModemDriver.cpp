// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "ModemDriver.h"
#include "trace.h"

using app::ModemDriver;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

os::StreamBuffer<char, ModemDriver::BUFFERSIZE> ModemDriver::InputBuffer;

void ModemDriver::ModemDriverInterruptHandler(uint8_t data)
{
#define MODEM_TX_DEBUG
#ifdef MODEM_TX_DEBUG
    //TODO: Remove this debug lines
    USART1->DR = (data & (uint16_t)0x01FF);
#endif
    if (InputBuffer.isFull()) {
        Trace(ZONE_ERROR, "ModemBuffer full. \r\n");
        return;
    }
    InputBuffer.sendFromISR(data);
}

ModemDriver::ModemDriver(const hal::UsartWithDma& interface,
                         const hal::Gpio&         resetPin,
                         const hal::Gpio&         powerPin,
                         const hal::Gpio&         supplyPin) :
    mModemTxTask("ModemTxTask",
                 ModemDriver::STACKSIZE,
                 os::Task::Priority::HIGH,
                 [this](const bool& join){
    modemTxTaskFunction(join);
}),
    mParserTask("ParserTask",
                ModemDriver::STACKSIZE,
                os::Task::Priority::VERY_HIGH,
                [this](const bool& join){
    parserTaskFunction(join);
}),
    mInterface(interface),
    mModemReset(resetPin),
    mModemPower(powerPin),
    mModemSupplyVoltage(supplyPin),
    mSend([&](std::string_view in, std::chrono::milliseconds timeout) -> size_t {
    return mInterface.send(in, timeout.count());
}),
    mRecv([&](uint8_t* output, const size_t length, std::chrono::milliseconds timeout) -> bool {
    return InputBuffer.receive(reinterpret_cast<char*>(output), length, timeout);
}),
    mParser(mRecv),
    mUrcCallbackReceive([&](const size_t socket, const size_t bytes){
    for (size_t i = 0; i < mNumOfSockets; i++) {
        auto sock = mSockets[i];
        if (sock->mSocket == socket) {
            if (bytes) {
                Trace(ZONE_INFO, "S%d: %d bytes available\r\n", socket, bytes);
                sock->mNumberOfBytesForReceive.overwrite(bytes);
            } else {
                sock->mNumberOfBytesForReceive.reset();
            }
        }
    }
}),
    mUrcCallbackClose([&](const size_t socket, const size_t bytes){
    Trace(ZONE_INFO, "Socket %d closed\r\n", socket);
    for (size_t i = 0; i < mNumOfSockets; i++) {
        auto sock = mSockets[i];
        if (sock->mSocket == socket) {
            sock->isOpen = false;
            sock->isCreated = false;
        }
    }
}),
    mATOK(),
    mATERROR(),
    mATUUSORF("UUSORF", "+UUSORF: ", mUrcCallbackReceive),
    mATUUSORD("UUSORD", "+UUSORD: ", mUrcCallbackReceive),
    mATUUPSDD("UUPSDD", "+UUPSDD: ", mUrcCallbackClose),
    mATUUSOCL("UUSOCL", "+UUSOCL: ", mUrcCallbackClose)
{
    mInterface.mUsart.enableNonBlockingReceive(ModemDriverInterruptHandler);

    mParser.registerAtCommand(&mATOK);
    mParser.registerAtCommand(&mATERROR);
    mParser.registerAtCommand(&mATUUSORF);
    mParser.registerAtCommand(&mATUUSORD);
    mParser.registerAtCommand(&mATUUPSDD);
    mParser.registerAtCommand(&mATUUSOCL);
}

ModemDriver::~ModemDriver(void)
{
    Trace(ZONE_ERROR, "Destructor shouldn't be called");
}

void ModemDriver::modemTxTaskFunction(const bool& join)
{
    constexpr const hal::Gpio& out = hal::Factory<hal::Gpio>::get<hal::Gpio::LED>();

    do {
        out = true;
        modemReset();

        if (!modemStartup()) {
            Trace(ZONE_VERBOSE, "ERROR modemStartup\r\n");
            continue;
        }

        while (mErrorCount < ERROR_THRESHOLD) {
            for (size_t i = 0; i < mNumOfSockets; i++) {
                auto sock = mSockets[i];
                if (!sock->isCreated) {
                    out = true;
                    sock->create();
                }

                if (sock->isCreated && !sock->isOpen) {
                    out = true;
                    sock->open();
                }

                if (!sock->isOpen) {
                    out = true;
                    handleError("0");
                    continue;
                }
                out = false;
                sock->checkAndSendData();
                sock->checkAndReceiveData();
            }
        }
    } while (!join);
}

void ModemDriver::parserTaskFunction(const bool& join)
{
    do {
        auto x = mParser.parse(std::chrono::milliseconds(35000));
        Trace(ZONE_INFO, "Parser terminated with %d\r\n", x);
    } while (!join);
}

bool ModemDriver::modemStartup(void)
{
    static std::array<app::ATCmd, 6> startupCommands = {
        app::ATCmd("ATZ", "ATZ\r", ""),
        app::ATCmd("ATE0V1", "ATE0V1\r", ""),
        app::ATCmd("AT+CMEE", "AT+CMEE=2\r", ""),
        app::ATCmd("AT+CGCLASS", "AT+CGCLASS=\"B\"\r", ""),
        app::ATCmd("AT+CGGATT", "AT+CGATT=1\r", ""),
        app::ATCmd("AT+UPSDA", "AT+UPSDA=0,3\r", ""),
    };

    for (auto& cmd : startupCommands) {
        os::ThisTask::sleep(std::chrono::milliseconds(100));

        cmd.mParser = &mParser;
        if (cmd.send(mSend, std::chrono::milliseconds(40000)) != AT::Return_t::FINISHED) {
            Trace(ZONE_VERBOSE, "Cmd %s ERROR\r\n", cmd.mName.data());
            return false;
        }
        Trace(ZONE_VERBOSE, "Cmd %s SUCCESS\r\n", cmd.mName.data());
    }
    return true;
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

void ModemDriver::modemReset(void)
{
    Trace(ZONE_INFO, "Modem Reset\r\n");
    modemOff();
    InputBuffer.reset();
    for (size_t i = 0; i < mNumOfSockets; i++) {
        auto sock = mSockets[i];
        sock->reset();
    }
    mErrorCount = 0;
    os::ThisTask::sleep(std::chrono::milliseconds(2000));
    modemOn();
    os::ThisTask::sleep(std::chrono::milliseconds(2000));
}

void ModemDriver::handleError(const char* str)
{
    Trace(ZONE_ERROR, "Error %s\r\n", str);
    if (mErrorCount >= ERROR_THRESHOLD) {
        mErrorCount = 0;
    }
    mErrorCount++;
}

app::Socket* ModemDriver::getSocket(app::Socket::Protocol protocol,
                                    std::string_view ip, std::string_view port)
{
    if (mNumOfSockets > MAXNUMOFSOCKETS) {
        Trace(ZONE_ERROR, "Maximum number of sockets reached\r\n");
        return nullptr;
    }

    app::Socket* sock = nullptr;
    if (protocol == Socket::Protocol::TCP) {
        sock = new TcpSocket(mParser, mSend, ip, port,
                             mUrcCallbackReceive, [] {
            Trace(ZONE_ERROR, "Error 1\r\n");
        });
    }

    if (protocol == Socket::Protocol::UDP) {
        sock = new UdpSocket(mParser, mSend, ip, port,
                             mUrcCallbackReceive, [] {
            Trace(ZONE_ERROR, "Error 2\r\n");
        });
    }

    if (protocol == Socket::Protocol::DNS) {
        sock = new DnsSocket(mParser, mSend, mUrcCallbackReceive, [&] {
            Trace(ZONE_ERROR, "Error 3\r\n");
        });
    }
    if (sock) {
        mSockets[mNumOfSockets++] = sock;
    }
    return sock;
}
