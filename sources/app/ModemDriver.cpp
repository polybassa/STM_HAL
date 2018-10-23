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

#include "ModemDriver.h"
#include "LockGuard.h"
#include "trace.h"
#include "AT_Parser.h"
#include "binascii.h"

using app::ModemDriver;

static const int __attribute__((unused)) g_DebugZones = 0; //ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

os::StreamBuffer<uint8_t, ModemDriver::BUFFERSIZE> ModemDriver::InputBuffer;
os::StreamBuffer<uint8_t, ModemDriver::BUFFERSIZE> ModemDriver::SendBuffer;
os::StreamBuffer<uint8_t, ModemDriver::BUFFERSIZE> ModemDriver::ReceiveBuffer;

void ModemDriver::ModemDriverInterruptHandler(uint8_t data)
{
    InputBuffer.sendFromISR(data);
}

ModemDriver::ModemDriver(const hal::UsartWithDma& interface,
                         const hal::Gpio&         resetPin,
                         const hal::Gpio&         powerPin,
                         const hal::Gpio&         supplyPin,
                         const Protocol           protocol) :
    os::DeepSleepModule(),
    mModemTxTask("ModemTxTask",
                 ModemDriver::STACKSIZE,
                 os::Task::Priority::HIGH,
                 [this](const bool& join)
                 {
                     modemTxTaskFunction(join);
                 }),
    mParserTask("ParserTask",
                ModemDriver::STACKSIZE,
                os::Task::Priority::VERY_HIGH,
                [this](const bool& join)
                {
                    parserTaskFunction(join);
                }),
    mInterface(interface),
    mModemReset(resetPin),
    mModemPower(powerPin),
    mModemSupplyVoltage(supplyPin),
    mSend([&](std::string_view in, std::chrono::milliseconds timeout)->size_t {
              os::ThisTask::sleep(std::chrono::milliseconds(60));
              return mInterface.send(in, timeout.count());
          }),
    mRecv([&](uint8_t * output, const size_t length, std::chrono::milliseconds timeout)->bool {
              return InputBuffer.receive(reinterpret_cast<char*>(output), length, timeout.count());
          }),
    mParser(mRecv),
    mNumberOfBytesForReceive(),
    mUrcCallbackReceive([&](size_t socket, size_t bytes)
                        {
                            Trace(ZONE_INFO, "%d bytes available\r\n", bytes);
                            if (bytes) {
                                mNumberOfBytesForReceive.overwrite(bytes);
                            }
                        }),
    mUrcCallbackClose([&](size_t socket, size_t bytes)
                      {
                          Trace(ZONE_INFO, "Socket closed\r\n");
                          // produce error to force TX-Task to restart modem
                          mErrorCount = ERROR_THRESHOLD;
                      }),

    mATUSOST(std::shared_ptr<app::ATCmdUSOST>(new app::ATCmdUSOST(mSend))),
    mATUSOWR(std::shared_ptr<app::ATCmdUSOWR>(new app::ATCmdUSOWR(mSend))),
    mATUSORF(std::shared_ptr<app::ATCmdUSORF>(new app::ATCmdUSORF(mSend, mUrcCallbackReceive))),
    mATUSORD(std::shared_ptr<app::ATCmdUSORD>(new app::ATCmdUSORD(mSend, mUrcCallbackReceive))),
    mATUUSORF(std::shared_ptr<app::ATCmdURC>(new app::ATCmdURC("UUSORF", "+UUSORF: ", mUrcCallbackReceive))),
    mATUUSORD(std::shared_ptr<app::ATCmdURC>(new app::ATCmdURC("UUSORD", "+UUSORD: ", mUrcCallbackReceive))),
    mATUUPSDD(std::shared_ptr<app::ATCmdURC>(new app::ATCmdURC("UUPSDD", "+UUPSDD: ", mUrcCallbackClose))),
    mATUUSOCL(std::shared_ptr<app::ATCmdURC>(new app::ATCmdURC("UUSOCL", "+UUSOCL: ", mUrcCallbackClose))),
    mATUPSND(std::shared_ptr<app::ATCmdUPSND>(new app::ATCmdUPSND(mSend))),
    mProtocol(protocol)
{
    Trace(ZONE_VERBOSE, "Constructor \r\n");

    mInterface.mUsart.enableNonBlockingReceive(ModemDriverInterruptHandler);

    mParser.registerAtCommand(std::shared_ptr<app::AT>(new app::ATCmdOK()));
    mParser.registerAtCommand(std::shared_ptr<app::AT>(new app::ATCmdERROR()));
    if ((mProtocol == Protocol::UDP) || (mProtocol == Protocol::DNS)) {
        mParser.registerAtCommand(mATUSOST);
    } else {
        mParser.registerAtCommand(mATUSOWR);
    }
    mParser.registerAtCommand(mATUSORF);
    mParser.registerAtCommand(mATUSORD);
    mParser.registerAtCommand(mATUUSORF);
    mParser.registerAtCommand(mATUUSORD);
    mParser.registerAtCommand(mATUUPSDD);
    mParser.registerAtCommand(mATUUSOCL);
    mParser.registerAtCommand(mATUPSND);
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
    os::ThisTask::sleep(std::chrono::milliseconds(500));

    do {
        modemReset();

        if (!modemStartup()) {
            Trace(ZONE_VERBOSE, "ERROR modemStartup\r\n");
            continue;
        }

        SendBuffer.send("ONLINE ", 6, std::chrono::milliseconds(100).count());

        while (mErrorCount < ERROR_THRESHOLD) {
            if (SendBuffer.bytesAvailable()) {
                switch (mProtocol) {
                case Protocol::UDP:
                    sendDataOverUdp();
                    break;

                case Protocol::TCP:
                    sendDataOverTcp();
                    break;

                case Protocol::DNS:
                    sendDataOverDns();
                    break;
                }
            }

            if ((mProtocol != Protocol::TCP) && (os::Task::getTickCount() - mTimeOfLastUdpSend >= 3000)) {
                SendBuffer.send("HELLO ", 6, std::chrono::milliseconds(100).count());
            }
            size_t bytes = 0;
            if (mNumberOfBytesForReceive.receive(bytes, std::chrono::milliseconds(10))) {
                switch (mProtocol) {
                case Protocol::UDP:
                    receiveDataOverUdp(bytes);
                    break;

                case Protocol::TCP:
                    receiveDataOverTcp(bytes);
                    break;

                case Protocol::DNS:
                    receiveDataOverDns(bytes);
                    break;
                }
            }
        }
    } while (!join);
}

void ModemDriver::parserTaskFunction(const bool& join)
{
    do {
        auto x = mParser.parse(std::chrono::milliseconds(15000));
        Trace(ZONE_INFO, "Parser terminated with %d\r\n", x);
    } while (!join);
}

bool ModemDriver::modemStartup(void)
{
    std::array<std::shared_ptr<app::ATCmd>, 8> startupCommands = {
        std::shared_ptr<app::ATCmd>(new app::ATCmd("ATZ", "ATZ\r", "")),
        std::shared_ptr<app::ATCmd>(new app::ATCmd("ATE0V1", "ATE0V1\r", "")),
        std::shared_ptr<app::ATCmd>(new app::ATCmd("AT+CMEE", "AT+CMEE=2\r", "")),
        std::shared_ptr<app::ATCmd>(new app::ATCmd("AT+CGCLASS", "AT+CGCLASS=\"B\"\r", "")),
        std::shared_ptr<app::ATCmd>(new app::ATCmd("AT+CGGATT", "AT+CGATT=1\r", "")),
        std::shared_ptr<app::ATCmd>(new app::ATCmd("AT+UPSDA", "AT+UPSDA=0,3\r", "")),
    };

    if (mProtocol == Protocol::TCP) {
        startupCommands[6] = std::shared_ptr<app::ATCmd>(new app::ATCmd("AT+USOCR", "AT+USOCR=6\r", ""));
    } else {
        startupCommands[6] = std::shared_ptr<app::ATCmd>(new app::ATCmd("AT+USOCR", "AT+USOCR=17\r", ""));
    }
    startupCommands[7] = std::shared_ptr<app::ATCmd>(new app::ATCmd("AT+USOCO", AT_CMD_USOCO, ""));

    for (const auto& cmd : startupCommands) {
        cmd->mParser = &mParser;

        os::ThisTask::sleep(std::chrono::milliseconds(2000));
        Trace(ZONE_VERBOSE, "Executing startup cmd\r\n");
        if ((cmd->send(mSend, std::chrono::milliseconds(16000)) != AT::Return_t::FINISHED) ||
            (cmd->wasExecutionSuccessful() == false))
        {
            Trace(ZONE_VERBOSE, "Executing startup cmd--> ERROR\r\n");
            return false;
        }
        Trace(ZONE_VERBOSE, "Executing startup cmd--> SUCCESS\r\n");
    }
    if (mProtocol == Protocol::DNS) {
        os::ThisTask::sleep(std::chrono::milliseconds(3000));
        this->getDns();
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
    ReceiveBuffer.reset();
    mNumberOfBytesForReceive.reset();
    mParser.reset();
    mErrorCount = 0;
    os::ThisTask::sleep(std::chrono::milliseconds(1000));
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

void ModemDriver::sendDataOverUdp(void)
{
    std::string tmpSendStr(SendBuffer.bytesAvailable(), '\x00');
    SendBuffer.receive(tmpSendStr.data(), tmpSendStr.length(), 1000);

    auto ret = mATUSOST->send(tmpSendStr, std::chrono::milliseconds(1000));

    if (ret == AT::Return_t::ERROR) {
        handleError();
    } else if (ret == AT::Return_t::FINISHED) {
        mTimeOfLastUdpSend = os::Task::getTickCount();
    }
    mATUSORF->send(0, std::chrono::milliseconds(1000));
}

void ModemDriver::sendDataOverTcp(void)
{
    std::string tmpSendStr(SendBuffer.bytesAvailable(), '\x00');
    SendBuffer.receive(tmpSendStr.data(), tmpSendStr.length(), 1000);

    auto ret = mATUSOWR->send(tmpSendStr, std::chrono::milliseconds(5000));

    if (ret == AT::Return_t::ERROR) {
        handleError();
    } else if (ret == AT::Return_t::FINISHED) {
        mTimeOfLastUdpSend = os::Task::getTickCount();
    }
    mATUSORD->send(0, std::chrono::milliseconds(1000));
}

void ModemDriver::sendDataOverDns(void)
{
    static constexpr const size_t MAX_PAYLOAD_LENGTH = 24;
    static short counter = 900;

    counter++;
    counter %= 999;
    std::string counterStr = std::to_string(counter);

    std::string tmpPayloadStr(std::max(SendBuffer.bytesAvailable(),
                                       MAX_PAYLOAD_LENGTH), '\x00');
    SendBuffer.receive(tmpPayloadStr.data(), tmpPayloadStr.length(), 1000);

    std::string hexPayloadStr;
    hexlify(hexPayloadStr, tmpPayloadStr);

    std::string tmpSendStr(
                           "\x00\x00\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00\x33\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x70\x63\x63\x63\x02\x74\x31\x05\x7a\x69\x6e\x6f\x6f\x02\x69\x74\x00\x00\x1c\x00\x01",
                           81);

    tmpSendStr.replace(13, hexPayloadStr.length(), hexPayloadStr);
    tmpSendStr.replace(64 - counterStr.length(), counterStr.length(), counterStr);

    auto ret = mATUSOST->send(0, mATUPSND->getData(), "53", tmpSendStr, std::chrono::milliseconds(1000));

    if (ret == AT::Return_t::ERROR) {
        handleError();
    } else if (ret == AT::Return_t::FINISHED) {
        mTimeOfLastUdpSend = os::Task::getTickCount();
    }
    mATUSORF->send(0, std::chrono::milliseconds(1000));
}

void ModemDriver::receiveDataOverUdp(size_t bytes)
{
    Trace(ZONE_INFO, "Start receive %d\r\n", bytes);
    if (bytes == 0) {
        return;
    }
    auto ret = mATUSORF->send(bytes, std::chrono::milliseconds(1000));
    if (ret == AT::Return_t::FINISHED) {
        storeReceivedData(mATUSORF->getData());
    } else {
        handleError();
    }
}

void ModemDriver::receiveDataOverTcp(size_t bytes)
{
    Trace(ZONE_INFO, "Start receive %d\r\n", bytes);
    if (bytes == 0) {
        return;
    }
    auto ret = mATUSORD->send(bytes, std::chrono::milliseconds(1000));
    if (ret == AT::Return_t::FINISHED) {
        storeReceivedData(mATUSORD->getData());
    } else {
        handleError();
    }
}

void ModemDriver::receiveDataOverDns(size_t bytes)
{
    Trace(ZONE_INFO, "Start receive over dns %d\r\n", bytes);
    if (bytes == 0) {
        return;
    }
    auto ret = mATUSORF->send(bytes, std::chrono::milliseconds(1000));
    if (ret == AT::Return_t::FINISHED) {
        const auto& data = mATUSORF->getData();

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
        handleError();
    }
}

void ModemDriver::storeReceivedData(const std::string& data)
{
    if (mReceiveCallback) {
        mReceiveCallback(data);
    } else {
        ReceiveBuffer.send(data.data(), data.length(), 1000);
    }
    Trace(ZONE_INFO, "Data stored\r\n");
}

size_t ModemDriver::send(std::string_view message, const uint32_t ticksToWait)
{
    if (SendBuffer.send(message.data(), message.length(), ticksToWait)) {
        return message.length();
    }
    return 0;
}

size_t ModemDriver::receive(uint8_t* message, size_t length, uint32_t ticksToWait)
{
    if (ReceiveBuffer.receive(reinterpret_cast<char*>(message), length, ticksToWait)) {
        return length;
    }
    return 0;
}

const std::string& ModemDriver::getDns(void)
{
    const auto ret = mATUPSND->send(0, 1, std::chrono::milliseconds(1000));

    if (ret == AT::Return_t::ERROR) {
        handleError();
    }
    return mATUPSND->getData();
}

size_t ModemDriver::bytesAvailable(void) const
{
    return ReceiveBuffer.bytesAvailable();
}

void ModemDriver::registerReceiveCallback(std::function<void(std::string_view)> f)
{
    mReceiveCallback = f;
}
void ModemDriver::unregisterReceiveCallback(void)
{
    mReceiveCallback = nullptr;
}
