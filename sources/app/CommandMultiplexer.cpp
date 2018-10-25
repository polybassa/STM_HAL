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

using app::CommandMultiplexer;
using app::Socket;

static const int __attribute__((used)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

CommandMultiplexer::CommandMultiplexer(std::shared_ptr<Socket> control,
                                       std::shared_ptr<Socket> data,
                                       CanController&          can,
                                       DemoExecuter&           demo) :
    os::DeepSleepModule(), mCommandMultiplexerTask("CommandMultiplexer",
                                                   CommandMultiplexer::STACKSIZE, os::Task::Priority::HIGH,
                                                   [this](const bool& join)
                                                   {
                                                       commandMultiplexerTaskFunction(join);
                                                   }),
    mCtrlSock(control), mDataSock(data), mCan(can), mDemo(demo)
{
    mCtrlSock->registerReceiveCallback([&](std::string_view cmd){
                                           multiplexCommand(cmd);
                                       });
    mDataSock->registerReceiveCallback([&](std::string_view cmd){
                                           mCan.send(cmd, 200);
                                       });
    mCan.registerReceiveCallback([&](std::string_view data){
                                     if (mCanRxEnabled) {
                                         mDataSock->send(data, 200);
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

void CommandMultiplexer::multiplexCommand(std::string_view input)
{
    auto ctrlindex = input.find('$');

    if (ctrlindex != std::string_view::npos) {
        if (input.length() - 2 <= ctrlindex) {
            return; // Empty command
        }
        SpecialCommand_t cmd = SpecialCommand_t(input[ctrlindex + 1]);
        handleSpecialCommand(cmd,
                             std::string_view(input.data() + ctrlindex + 2,
                                              input.length() - ctrlindex - 2));
    }
}

void CommandMultiplexer::handleSpecialCommand(CommandMultiplexer::SpecialCommand_t cmd, std::string_view data)
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

void CommandMultiplexer::updateRemoteCode(std::string_view code)
{
    uint8_t* p = (uint8_t*)(&_rce_start);
    uint8_t* end = (uint8_t*)(&_rce_end);
    for (size_t i = 0; i < code.length() && p < end; i++) {
        *p++ = code.data()[i];
        Trace(ZONE_INFO, "%02X \r\n", *(p - 1));
    }
}

void CommandMultiplexer::commandMultiplexerTaskFunction(const bool& join)
{
    Trace(ZONE_INFO, "Start command multiplexer \r\n");

    do {
        os::ThisTask::sleep(std::chrono::seconds(10));
    } while (!join);
}
