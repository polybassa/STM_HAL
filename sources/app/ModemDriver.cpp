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
#include "AT_Cmd.h"

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
    mInterface(interface),
    mModemReset(resetPin),
    mModemPower(powerPin),
    mModemSupplyVoltage(supplyPin)
{
    mInterface.mUsart.enableNonBlockingReceive(ModemDriverInterruptHandler);

    mSend = [&](std::string_view in, std::chrono::milliseconds timeout)->size_t {
        return mInterface.send(in, timeout.count());
    };

    mRecv = [&](uint8_t * output, const size_t length, std::chrono::milliseconds timeout)->bool {
        return InputBuffer.receive(reinterpret_cast<char*>(output), length, timeout.count());
    };
}

void ModemDriver::enterDeepSleep(void)
{
    mModemTxTask.join();
}

void ModemDriver::exitDeepSleep(void)
{
    mModemTxTask.start();
}

void ModemDriver::modemTxTaskFunction(const bool& join)
{
    do {
        if (!modemStartup()) {
            continue;
        }

        uint32_t timeOfLastUdpSend = 0;

        app::ReceiveAtCmd receiveCmd(mSend, mRecv, std::chrono::milliseconds(700));
        app::SendAtCmd sendCmd(mSend, mRecv, std::chrono::milliseconds(1000));

        while (mErrorCount < ERROR_THRESHOLD) {
            if (SendBuffer.bytesAvailable()) {
                std::string tmpSendStr(SendBuffer.bytesAvailable(), '\x00');
                SendBuffer.receive(tmpSendStr.data(), tmpSendStr.length(), 1000);
                InputBuffer.reset();
                if (!sendCmd.process(tmpSendStr)) {
                    handleError();
                } else {
                    timeOfLastUdpSend = os::Task::getTickCount();
                }
            }

            if (os::Task::getTickCount() - timeOfLastUdpSend >= 1000) {
                InputBuffer.reset();
                if (!sendCmd.process("HELLO")) {
                    handleError();
                } else {
                    timeOfLastUdpSend = os::Task::getTickCount();
                }
            } else {
                // wait a second for new input data
                std::string receiveString;
                if (receiveCmd.process(receiveString, false)) {
                    if (mReceiveCallback) {
                        mReceiveCallback(receiveString);
                    }
                    ReceiveBuffer.send(receiveString.data(), receiveString.length(), 1000);
                    Trace(ZONE_INFO, "Data received\r\n");
                }
            }
        }
    } while (!join);
}

bool ModemDriver::modemStartup(void)
{
    modemReset();

    std::array<app::BasicAtCmd, 7> StartupCommands =
    {{
         app::BasicAtCmd(std::string_view("ATE0V1\r"), mSend, mRecv, std::chrono::milliseconds(10000)),
         app::BasicAtCmd(std::string_view("AT+CMEE=2\r"), mSend, mRecv, std::chrono::milliseconds(10000)),
         app::BasicAtCmd(std::string_view("AT+CGCLASS=\"B\"\r"), mSend, mRecv, std::chrono::milliseconds(10000)),
         app::BasicAtCmd(std::string_view("AT+CGATT=1\r"), mSend, mRecv, std::chrono::milliseconds(10000)),
         app::BasicAtCmd(std::string_view("AT+UPSDA=0,3\r"), mSend, mRecv, std::chrono::milliseconds(10000)),
         app::BasicAtCmd(std::string_view("AT+USOCR=17\r"), mSend, mRecv, std::chrono::milliseconds(10000)),
         app::BasicAtCmd(std::string_view(AT_CMD_USOCO), mSend, mRecv, std::chrono::milliseconds(10000))
     }};

    for (auto& cmd : StartupCommands) {
        if (!cmd.process()) {
            return false;
        }
        InputBuffer.reset();
        os::ThisTask::sleep(std::chrono::milliseconds(2000));
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
    InputBuffer.reset();

    if (mErrorCount == ERROR_THRESHOLD) {
        mErrorCount = 0;
    }
    mErrorCount++;
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

void ModemDriver::registerReceiveCallback(std::function<void(std::string_view)> f)
{
    mReceiveCallback = f;
}
void ModemDriver::unregisterReceiveCallback(void)
{
    mReceiveCallback = nullptr;
}
