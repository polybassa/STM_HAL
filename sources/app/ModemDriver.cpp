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
                         const hal::Gpio&         supplyPin) :
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
                os::Task::Priority::HIGH,
                [this](const bool& join)
                {
                    parserTaskFunction(join);
                }),
    mInterface(interface),
    mModemReset(resetPin),
    mModemPower(powerPin),
    mModemSupplyVoltage(supplyPin),
    mSend([&](std::string_view in, std::chrono::milliseconds timeout)->size_t {
              return mInterface.send(in, timeout.count());
          }),
    mRecv([&](uint8_t * output, const size_t length, std::chrono::milliseconds timeout)->bool {
              return InputBuffer.receive(reinterpret_cast<char*>(output), length, timeout.count());
          }),
    mParser(mRecv),
    mATUSOST(std::shared_ptr<app::ATCmdUSOST>(new app::ATCmdUSOST(mSend, mParser))),
    mATUSORF(std::shared_ptr<app::ATCmdUSORF>(new app::ATCmdUSORF(mSend, mParser)))
{
    mInterface.mUsart.enableNonBlockingReceive(ModemDriverInterruptHandler);

    mParser.registerAtCommand(std::shared_ptr<app::AT>(new app::ATCmdOK(mParser)));
    mParser.registerAtCommand(std::shared_ptr<app::AT>(new app::ATCmdERROR(mParser)));

    std::function<void(size_t, size_t)> urcCallback = [this](size_t socket, size_t bytes)
    {
        this->mNumberOfBytesForReceive.overwrite(bytes);
    };

    auto atuusorf = std::shared_ptr<app::ATCmdURC>(new app::ATCmdURC("UUSORF", "+UUSORF: ", mParser, urcCallback));
    auto atuusord = std::shared_ptr<app::ATCmdURC>(new app::ATCmdURC("UUSORD", "+UUSORD: ", mParser, urcCallback));

    mParser.registerAtCommand(mATUSOST);
    mParser.registerAtCommand(mATUSORF);
    mParser.registerAtCommand(atuusorf);
    mParser.registerAtCommand(atuusord);
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
            continue;
        }

        while (mErrorCount < ERROR_THRESHOLD) {
            if (SendBuffer.bytesAvailable()) {
                sendData();
            }

            if (os::Task::getTickCount() - mTimeOfLastUdpSend >= 1000) {
                SendBuffer.send("HELLO ", 6, std::chrono::milliseconds(100).count());
            }
            size_t bytes = 0;
            if (mNumberOfBytesForReceive.receive(bytes, std::chrono::milliseconds(0))) {
                receiveData(bytes);
            }
        }
    } while (!join);
}

void ModemDriver::parserTaskFunction(const bool& join)
{
    do {
        auto x = mParser.parse(std::chrono::milliseconds(10000));
        Trace(ZONE_INFO, "Parser terminated with %d\r\n", x);
    } while (!join);
}

bool ModemDriver::modemStartup(void)
{
    std::array<std::shared_ptr<app::ATCmd>, 7> startupCommands = {
        std::shared_ptr<app::ATCmd>(new app::ATCmd("ATE0V1", "ATE0V1\r", "", mParser)),
        std::shared_ptr<app::ATCmd>(new app::ATCmd("AT+CMEE", "AT+CMEE=2\r", "", mParser)),
        std::shared_ptr<app::ATCmd>(new app::ATCmd("AT+CGCLASS", "AT+CGCLASS=\"B\"\r", "", mParser)),
        std::shared_ptr<app::ATCmd>(new app::ATCmd("AT+CGGATT", "AT+CGATT=1\r", "", mParser)),
        std::shared_ptr<app::ATCmd>(new app::ATCmd("AT+UPSDA", "AT+UPSDA=0,3\r", "", mParser)),
        std::shared_ptr<app::ATCmd>(new app::ATCmd("AT+USOCR", "AT+USOCR=17\r", "", mParser)),
        std::shared_ptr<app::ATCmd>(new app::ATCmd("AT+USOCO", AT_CMD_USOCO, "", mParser)),
    };

    for (const auto& cmd : startupCommands) {
        os::ThisTask::sleep(std::chrono::milliseconds(2000));
        if ((cmd->send(mSend, std::chrono::milliseconds(10000)) != AT::Return_t::FINISHED) ||
            (cmd->wasExecutionSuccessful() == false))
        {
            return false;
        }
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
    modemOff();
    InputBuffer.reset();
    mErrorCount = 0;
    os::ThisTask::sleep(std::chrono::milliseconds(1000));
    modemOn();
    os::ThisTask::sleep(std::chrono::milliseconds(1000));
}

void ModemDriver::handleError(void)
{
    Trace(ZONE_ERROR, "Error\r\n");
    if (mErrorCount >= ERROR_THRESHOLD) {
        mErrorCount = 0;
    }
    mErrorCount++;
}

void ModemDriver::sendData(void)
{
    std::string tmpSendStr(SendBuffer.bytesAvailable(), '\x00');
    SendBuffer.receive(tmpSendStr.data(), tmpSendStr.length(), 1000);

    auto ret = mATUSOST->send(tmpSendStr, std::chrono::milliseconds(1000));

    if (ret == AT::Return_t::ERROR) {
        handleError();
    } else if (ret == AT::Return_t::FINISHED) {
        mTimeOfLastUdpSend = os::Task::getTickCount();
    }
}

void ModemDriver::receiveData(size_t bytes)
{
    auto ret = mATUSORF->send(bytes, std::chrono::milliseconds(1000));
    if (ret == AT::Return_t::FINISHED) {
        if (mReceiveCallback) {
            mReceiveCallback(mATUSORF->getData());
        } else {
            ReceiveBuffer.send(mATUSORF->getData().data(), mATUSORF->getData().length(), 1000);
        }
        Trace(ZONE_INFO, "Data received\r\n");
    }
}

size_t ModemDriver::send(std::string_view message, const uint32_t ticksToWait) const
{
    if (SendBuffer.send(message.data(), message.length(), ticksToWait)) {
        return message.length();
    }
    return 0;
}

size_t ModemDriver::receive(uint8_t* message, size_t length, uint32_t ticksToWait) const
{
    if (ReceiveBuffer.receive(reinterpret_cast<char*>(message), length, ticksToWait)) {
        return length;
    }
    return 0;
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
