/* Copyright (C) 2016 Nils Weiss
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

#include "CommandMultiplexer.h"
#include "trace.h"
#include <cstring>

using app::CommandMultiplexer;
using app::Socket;

static const int __attribute__((used)) g_DebugZones = 0; // ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

CommandMultiplexer::CommandMultiplexer(std::shared_ptr<Socket> control,
                                       std::shared_ptr<Socket> data,
                                       CanController&          can,
                                       DemoExecuter&           demo) :
    os::DeepSleepModule(), mCommandMultiplexerTask("CommandMultiplexer",
                                                   CommandMultiplexer::STACKSIZE, os::Task::Priority::MEDIUM,
                                                   [&](const bool& join)
{
    commandMultiplexerTaskFunction(join);
}),
    mCtrlSock(control), mDataSock(data), mCan(can), mDemo(demo)
{
    mDataSock->registerReceiveCallback([&](const std::string_view cmd){
        mCan.send(cmd, 1000);
    });
    mCan.registerReceiveCallback([&](const std::string_view data){
        if (mCanRxEnabled) {
            mDataSock->send(data, 1000);
        }
    });
}

void CommandMultiplexer::enterDeepSleep(void)
{
    mCommandMultiplexerTask.join();
}

void CommandMultiplexer::exitDeepSleep(void)
{
    mCommandMultiplexerTask.start();
}

void CommandMultiplexer::multiplexCommand(const std::string_view input)
{
    Trace(ZONE_INFO, "Got cmd\r\n");
    const auto ctrlindex = input.find('$');

    if (ctrlindex != std::string_view::npos) {
        if (input.length() - 2 <= ctrlindex) {
            Trace(ZONE_INFO, "Empty command received.\r\n");
            return;
        }
        SpecialCommand_t cmd = SpecialCommand_t(input[ctrlindex + 1]);
        handleSpecialCommand(cmd,
                             std::string_view(input.data() + ctrlindex + 2,
                                              input.length() - ctrlindex - 2));
    } else {
        showHelp();
    }
}

void CommandMultiplexer::showHelp(void) const
{
    mCtrlSock->send("--------CARSEC Dongle ---------\r\n");
    mCtrlSock->send("\r\n");
    mCtrlSock->send("Commands:\r\n");
    mCtrlSock->send("$0x =  RUN_DEMO x\r\n");
    mCtrlSock->send("$1  =  FLASH_CAN_MCU\r\n");
    mCtrlSock->send("$2  =  CAN_ON\r\n");
    mCtrlSock->send("$3  =  CAN_OFF\r\n");
    mCtrlSock->send("$4  =  ENABLE_CAN_RX\r\n");
    mCtrlSock->send("$5  =  DISABLE_CAN_RX\r\n");
    mCtrlSock->send("$6  =  DONGLE_RESET\r\n");
    mCtrlSock->send("$7  =  RC_UPDATE\r\n");
    mCtrlSock->send("$8  =  RC_EXECUTE\r\n");
    os::ThisTask::sleep(std::chrono::milliseconds(500));
}

void CommandMultiplexer::handleSpecialCommand(CommandMultiplexer::SpecialCommand_t cmd, const std::string_view data)
{
    switch (cmd) {
    case SpecialCommand_t::FLASH_CAN_MCU:
        Trace(ZONE_INFO, "Flash CAN MCU requested.\r\n");
        mCan.triggerFirmwareUpdate();
        mCtrlSock->send("$Triggerd FW Update\r\n");
        break;

    case SpecialCommand_t::CAN_ON:
        Trace(ZONE_INFO, "CAN ON requested.\r\n");
        mCan.on();
        mCtrlSock->send("$CAN ON\r\n");
        break;

    case SpecialCommand_t::CAN_OFF:
        Trace(ZONE_INFO, "CAN OFF requested.\r\n");
        mCan.off();
        mCtrlSock->send("$CAN OFF\r\n");
        break;

    case SpecialCommand_t::ENABLE_CAN_RX:
        Trace(ZONE_INFO, "Enable CAN RX requested.\r\n");
        mCanRxEnabled = true;
        mCtrlSock->send("$CAN RX on\r\n");
        break;

    case SpecialCommand_t::DISABLE_CAN_RX:
        Trace(ZONE_INFO, "Disable CAN RX requested.\r\n");
        mCanRxEnabled = false;
        mCtrlSock->send("$CAN RX off\r\n");
        break;

    case SpecialCommand_t::RUN_DEMO:
        Trace(ZONE_INFO, "Run demo requested.\r\n");
        mCtrlSock->send("$RUN DEMO\r\n");
        mDemo.runDemo(data);
        break;

    case SpecialCommand_t::DONGLE_RESET:
        Trace(ZONE_INFO, "Reset myself... bye.bye!\r\n");
        mCtrlSock->send("Reset myself... bye.bye!\r\n");
        os::ThisTask::sleep(std::chrono::milliseconds(500));
        NVIC_SystemReset();
        break;

    case SpecialCommand_t::RC_EXECUTE:
        Trace(ZONE_INFO, "EXECUTE Remote Code.\r\n");
        mCtrlSock->send("Run Remote Code!\r\n");
        remoteCodeExecution();
        break;

    case SpecialCommand_t::RC_UPDATE:
        Trace(ZONE_INFO, "Update Remote Code.\r\n");
        mCtrlSock->send("Update Remote Code!\r\n");
        updateRemoteCode(data);
        break;

    default:
        Trace(ZONE_INFO, "Unknown special command '%c'\r\n", cmd);
        break;
    }
}

__attribute__ ((section(".rce.str"))) uint8_t str[] = "hello from RCE\r\n";
__attribute__ ((section(".rce"))) void CommandMultiplexer::remoteCodeExecution(void)
{
    constexpr const hal::Usart& debug = hal::Factory<hal::Usart>::get<hal::Usart::DEBUG_IF>();
    debug.send(str, sizeof(str));
    mCtrlSock->send(std::string_view(reinterpret_cast<const char*>(str), sizeof(str)), 100);
    os::ThisTask::sleep(std::chrono::milliseconds(1000));
    mCtrlSock->send(std::string_view(reinterpret_cast<const char*>(str), sizeof(str)), 100);
}

extern "C" char _rce_start;
extern "C" char _rce_end;

void CommandMultiplexer::updateRemoteCode(const std::string_view code)
{
    if (code.length() <= 2) {
        Trace(ZONE_ERROR, "Code to short to contain a 2 byte length field\r\n");
        return;
    }
    size_t length;

    std::memcpy(&length, code.data(), sizeof(length));

    uint8_t* p = (uint8_t*)(&_rce_start);
    uint8_t const* const end = (uint8_t*)(&_rce_end);

    if (length > static_cast<size_t>(end - p)) {
        Trace(ZONE_ERROR, "Provide a valid length smaller than %d \r\n", (end - p));
        return;
    }

    std::string_view segment(code.data() + 2, code.length() - 2);

    do {
        if (p + segment.length() > end) {
            Trace(ZONE_ERROR, "Memory would overflow \r\n");
            return;
        }

        if (segment.length() > length) {
            Trace(ZONE_ERROR, "More bytes in segment than I'm waiting for\r\n");
            return;
        }

        std::memcpy(p, segment.data(), segment.length());
        p += segment.length();
        length -= segment.length();

        const size_t newSegmentLength = mCtrlSock->receive(
                                                           reinterpret_cast<uint8_t*>(mCommandBuffer.data()),
                                                           mCommandBuffer.size(), 2000);
        if (!newSegmentLength) {
            Trace(ZONE_ERROR, "Timeout while waiting for more data \r\n");
            return;
        }
        segment = std::string_view(mCommandBuffer.data(), newSegmentLength);
    } while (p < end && length);
}

void CommandMultiplexer::commandMultiplexerTaskFunction(const bool& join)
{
    Trace(ZONE_INFO, "Start command multiplexer \r\n");

    do {
        const auto length =
            mCtrlSock->receive(reinterpret_cast<uint8_t*>(mCommandBuffer.data()), mCommandBuffer.size());
        if (length) {
            multiplexCommand(std::string_view(mCommandBuffer.data(), length));
        }
    } while (!join);
}
