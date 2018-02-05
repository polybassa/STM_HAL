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

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

os::StreamBuffer<uint8_t, ModemDriver::BUFFERSIZE> ModemDriver::InputBuffer;
os::StreamBuffer<uint8_t, ModemDriver::BUFFERSIZE> ModemDriver::OutputBuffer;

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

    mRecv = [&](uint8_t & output, std::chrono::milliseconds timeout)->bool {
        return InputBuffer.receive(output, timeout.count());
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

        app::ReceiveAtCmd receiveCmd(mSend, mRecv, std::chrono::seconds(1));
        app::SendAtCmd sendCmd(mSend, mRecv, std::chrono::milliseconds(1000));

        while (mErrorCount < ERROR_THRESHOLD) {
            if (os::Task::getTickCount() - timeOfLastUdpSend >= 1000) {
                InputBuffer.reset();
                if (!sendCmd.process("HELLO")) {
                    handleError();
                } else {
                    timeOfLastUdpSend = os::Task::getTickCount();
                }
            } else {
                // wait a second for new input data
                Trace(ZONE_INFO, "SLEEP until RECV\r\n");
                if (receiveCmd.process(mDataString, false)) {
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
        os::ThisTask::sleep(std::chrono::milliseconds(1000));
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
