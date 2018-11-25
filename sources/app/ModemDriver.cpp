// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "ModemDriver.h"
#include "trace.h"
#include <memory>

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
    InputBuffer.sendFromISR(data);
}

ModemDriver::ModemDriver(const hal::UsartWithDma& interface,
                         const hal::Gpio&         resetPin,
                         const hal::Gpio&         powerPin,
                         const hal::Gpio&         supplyPin) :
    os::DeepSleepModule(),
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
    for (auto& sock: mSockets) {
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
    for (auto& sock: mSockets) {
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

void ModemDriver::enterDeepSleep(void)
{
    modemOff();
    mModemTxTask.join();
    mParserTask.join();
}

void ModemDriver::exitDeepSleep(void)
{
    mParserTask.start();
    mModemTxTask.start();
}

void ModemDriver::modemTxTaskFunction(const bool& join)
{
    do {
        modemReset();

        if (!modemStartup()) {
            Trace(ZONE_VERBOSE, "ERROR modemStartup\r\n");
            continue;
        }

        while (mErrorCount < ERROR_THRESHOLD) {
            for (auto& sock : mSockets) {
                if (!sock->isCreated) {
                    sock->create();
                }

                if (sock->isCreated && !sock->isOpen) {
                    sock->open();
                }

                if (!sock->isOpen) {
                    handleError();
                    continue;
                }

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
    for (auto& sock : mSockets) {
        sock->reset();
    }
    mErrorCount = 0;
    os::ThisTask::sleep(std::chrono::milliseconds(2000));
    modemOn();
    os::ThisTask::sleep(std::chrono::milliseconds(2000));
}

void ModemDriver::handleError(void)
{
    Trace(ZONE_ERROR, "Error\r\n");
    if (mErrorCount >= ERROR_THRESHOLD) {
        mErrorCount = 0;
    }
    mErrorCount++;
}

std::shared_ptr<app::Socket> ModemDriver::getSocket(app::Socket::Protocol protocol,
                                                    std::string_view ip, std::string_view port)
{
    std::shared_ptr<app::Socket> sock;
    if (protocol == Socket::Protocol::TCP) {
        sock = std::make_shared<TcpSocket>(mParser, mSend, ip, port,
                                           mUrcCallbackReceive, [&] {
            handleError();
        });
    }

    if (protocol == Socket::Protocol::UDP) {
        sock = std::make_shared<UdpSocket>(mParser, mSend, ip, port,
                                           mUrcCallbackReceive, [&] {
            handleError();
        });
    }

    if (protocol == Socket::Protocol::DNS) {
        sock = std::make_shared<DnsSocket>(mParser, mSend, mUrcCallbackReceive, [&] {
            handleError();
        });
    }
    if (sock) {
        mSockets.push_back(sock);
    }
    return sock;
}
