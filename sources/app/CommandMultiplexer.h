// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

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
