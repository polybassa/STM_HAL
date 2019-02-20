// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include <string>
#include <string_view>
#include <array>
#include <memory>
#include "TaskInterruptable.h"
#include "os_StreamBuffer.h"
#include "os_Queue.h"
#include "UsartWithDma.h"
#include "Gpio.h"
#include "AT_Parser.h"
#include "Socket.h"

namespace app
{
class ModemDriver final
{
    static constexpr size_t STACKSIZE = 3056;
    static constexpr size_t BUFFERSIZE = 1024;
    static constexpr size_t ERROR_THRESHOLD = 20;
    static os::StreamBuffer<char, BUFFERSIZE> InputBuffer;

    std::vector<std::shared_ptr<Socket> > mSockets;

    os::TaskInterruptable mModemTxTask;
    os::TaskInterruptable mParserTask;

    const hal::UsartWithDma& mInterface;
    const hal::Gpio& mModemReset;
    const hal::Gpio& mModemPower;
    const hal::Gpio& mModemSupplyVoltage;

    AT::SendFunction mSend;
    AT::ReceiveFunction mRecv;
    ATParser mParser;
    std::function<void(size_t, size_t)> mUrcCallbackReceive;
    std::function<void(size_t, size_t)> mUrcCallbackClose;

    app::ATCmdOK mATOK;
    app::ATCmdERROR mATERROR;
    app::ATCmdURC mATUUSORF;
    app::ATCmdURC mATUUSORD;
    app::ATCmdURC mATUUPSDD;
    app::ATCmdURC mATUUSOCL;

    size_t mErrorCount = 0;

    void modemTxTaskFunction(const bool&);
    void parserTaskFunction(const bool&);

    void modemOn(void) const;
    void modemOff(void) const;
    void modemReset(void);
    bool modemStartup(void);

    void handleError(const char* str = "");

public:
    ModemDriver(const hal::UsartWithDma& interface,
                const hal::Gpio&         resetPin,
                const hal::Gpio&         powerPin,
                const hal::Gpio&         supplyPin);

    ModemDriver(const ModemDriver&) = delete;
    ModemDriver(ModemDriver&&) = delete;
    ModemDriver& operator=(const ModemDriver&) = delete;
    ModemDriver& operator=(ModemDriver&&) = delete;
    ~ModemDriver(void);

    static void ModemDriverInterruptHandler(uint8_t);

    std::shared_ptr<Socket> getSocket(Socket::Protocol,
                                      std::string_view ip, std::string_view port);
};
}
