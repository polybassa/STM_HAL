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
using app::ModemDriver;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR |
                                                        ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

CommandMultiplexer::CommandMultiplexer(ModemDriver& modem, CanController& can) :
    os::DeepSleepModule(), mCommandMultiplexerTask("CommandMultiplexer",
                                                   CommandMultiplexer::STACKSIZE, os::Task::Priority::HIGH,
                                                   [this](const bool& join)
                                                   {
                                                       commandMultiplexerTaskFunction(join);
                                                   }), mModem(modem), mCan(can)
{
    mModem.registerReceiveCallback([&](std::string_view cmd){
                                       multiplexCommand(cmd);
                                   });
    mCan.registerReceiveCallback([&](std::string_view data){
                                     mModem.send(data, 200);
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
    uint8_t packetType = input[0];

    if ((packetType == 1) || (packetType == '/')) {
        if (input.length() <= 1) {
            return; // Empty command
        }
        SpecialCommand_t cmd = SpecialCommand_t(input[1]);
        handleSpecialCommand(cmd, std::string_view(input.data() + 2, input.length() - 2));
    } else {
        /* Anything else is forwarded to SecCo */
        mCan.send(input, 10);
    }
}

void CommandMultiplexer::handleSpecialCommand(CommandMultiplexer::SpecialCommand_t cmd, std::string_view data)
{
    switch (cmd) {
    case SpecialCommand_t::FLASH_CAN_MCU:
        Trace(ZONE_INFO, "Flash CAN MCU requested.\r\n");
        //flashSecCoFirmware(&huart2);
        break;

    case SpecialCommand_t::CAN_ON:
        Trace(ZONE_INFO, "CAN ON requested.\r\n");
        mCan.on();
        break;

    case SpecialCommand_t::CAN_OFF:
        Trace(ZONE_INFO, "CAN OFF requested.\r\n");
        mCan.off();
        break;

    case SpecialCommand_t::ENABLE_CAN_RX:
        Trace(ZONE_INFO, "Enable CAN RX requested.\r\n");
        //rxFromCanEnabled = 1;
        break;

    case SpecialCommand_t::DISABLE_CAN_RX:
        Trace(ZONE_INFO, "Disable CAN RX requested.\r\n");
        //rxFromCanEnabled = 0;
        break;

    case SpecialCommand_t::RUN_DEMO:
        Trace(ZONE_INFO, "Run demo requested.\r\n");
        //runDemo(&huart2, (const char*)data);
        break;

    case SpecialCommand_t::DONGLE_RESET:
        Trace(ZONE_INFO, "Reset myself... bye.bye!\r\n");
        NVIC_SystemReset();
        break;

    case SpecialCommand_t::RC_EXECUTE:
        Trace(ZONE_INFO, "EXECUTE Remote Code.\r\n");
        remoteCodeExecution();
        break;

    case SpecialCommand_t::RC_UPDATE:
        Trace(ZONE_INFO, "Update Remote Code.\r\n");
        updateRemoteCode(data);
        break;

    default:
        Trace(ZONE_INFO, "Unknown special command '%c'\r\n", cmd);
        break;
    }
}

__attribute__ ((section(".rce.str"))) uint8_t str[] = "hello from RCE";
__attribute__ ((section(".rce"))) void CommandMultiplexer::remoteCodeExecution(void)
{
    constexpr const hal::Usart& debug = hal::Factory<hal::Usart>::get<hal::Usart::DEBUG_IF>();
    debug.send(str, sizeof(str));
}

extern "C" char _rce_start;
extern "C" char _rce_end;

void CommandMultiplexer::updateRemoteCode(std::string_view code)
{
    uint8_t* p = (uint8_t*)(&_rce_start);
    uint8_t* end = (uint8_t*)(&_rce_end);
    for (int i = 0; i < code.length() && p < end; i++) {
        *p++ = code.data()[i];
        Trace(ZONE_INFO, "%02X \r\n", *(p - 1));
    }
}

void CommandMultiplexer::commandMultiplexerTaskFunction(const bool& join)
{
    Trace(ZONE_INFO, "Start command multiplexer \r\n");

    do {
        os::ThisTask::sleep(std::chrono::seconds(1));
    } while (!join);
}
