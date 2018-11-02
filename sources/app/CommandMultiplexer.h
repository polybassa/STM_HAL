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

#ifndef SOURCES_PMD_COMMANDMULTIPLEXER_H_
#define SOURCES_PMD_COMMANDMULTIPLEXER_H_

#include <string_view>
#include <memory>
#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "Socket.h"
#include "CanController.h"
#include "DemoExecuter.h"

namespace app
{
class CommandMultiplexer final :
    private os::DeepSleepModule
{
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr uint32_t STACKSIZE = 1024;
    static constexpr size_t MAXCOMMANDSIZE = 64;
    std::array<char, MAXCOMMANDSIZE> mCommandBuffer;

    enum class SpecialCommand_t {
        RUN_DEMO = '0',
        FLASH_CAN_MCU,
        CAN_ON,
        CAN_OFF,
        ENABLE_CAN_RX,
        DISABLE_CAN_RX,
        DONGLE_RESET,
        RC_UPDATE,
        RC_EXECUTE
    };

    os::TaskInterruptable mCommandMultiplexerTask;
    std::shared_ptr<Socket> mCtrlSock;
    std::shared_ptr<Socket> mDataSock;
    CanController& mCan;
    DemoExecuter& mDemo;
    bool mCanRxEnabled = false;

    void multiplexCommand(const std::string_view cmd);
    void handleSpecialCommand(SpecialCommand_t cmd, const std::string_view);

    void commandMultiplexerTaskFunction(const bool&);
    void remoteCodeExecution(void);
    void updateRemoteCode(const std::string_view code);
    void showHelp(void) const;

public:
    CommandMultiplexer(std::shared_ptr<Socket> control,
                       std::shared_ptr<Socket> data,
                       CanController&          can,
                       DemoExecuter&           demo);

    CommandMultiplexer(const CommandMultiplexer&) = delete;
    CommandMultiplexer(CommandMultiplexer&&) = delete;
    CommandMultiplexer& operator=(const CommandMultiplexer&) = delete;
    CommandMultiplexer& operator=(CommandMultiplexer&&) = delete;
};
}

#endif /* SOURCES_PMD_COMMANDMULTIPLEXER_H_ */
