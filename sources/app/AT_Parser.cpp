// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "AT_Parser.h"
#include "trace.h"
#include "binascii.h"
#include "LockGuard.h"
#include <cstring>

using app::AT;
using app::ATCmd;
using app::ATCmdERROR;
using app::ATCmdOK;
using app::ATCmdRXData;
using app::ATCmdTX;
using app::ATCmdUPSND;
using app::ATCmdURC;
using app::ATCmdUSOCO;
using app::ATCmdUSOCR;
using app::ATCmdUSOCTL;
using app::ATCmdUSORD;
using app::ATCmdUSORF;
using app::ATCmdUSOSO;
using app::ATCmdUSOST;
using app::ATCmdUSOWR;
using app::ATParser;

static constexpr const int __attribute__((unused)) g_DebugZones = 0; //ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

//------------------------ATCmd---------------------------------

AT::Return_t ATCmd::send(AT::SendFunction& sendFunction, const std::chrono::milliseconds timeout)
{
    if (!mParser) {
        Trace(ZONE_ERROR, "Parser not set\n");
        return Return_t::ERROR;
    }

    mSendResult.reset();

    {
        os::LockGuard<os::Mutex> lock(mParser->mWaitingCmdMutex);

        if (mParser->mWaitingCmd) {
            Trace(ZONE_VERBOSE, "Parser not ready\n");
            return Return_t::TRY_AGAIN;
        }
        Trace(ZONE_VERBOSE, "sending: %s\r\n", mRequest.data());

        mParser->mWaitingCmd = this;

        if (sendFunction(mRequest, timeout) != mRequest.length()) {
            Trace(ZONE_ERROR, "Couldn't send\n");
            mParser->mWaitingCmd = nullptr;
            return Return_t::ERROR;
        }
        Trace(ZONE_VERBOSE, "waiting\r\n");
    }
    bool commandSuccess = false;
    if (mSendResult.receive(commandSuccess, timeout) && commandSuccess) {
        Trace(ZONE_VERBOSE, "done\r\n");
        return Return_t::FINISHED;
    }
    {
        os::LockGuard<os::Mutex> lock(mParser->mWaitingCmdMutex);
        Trace(ZONE_VERBOSE, "Timeout: %s\r\n", mRequest.data());
        mParser->mWaitingCmd = nullptr;
        return Return_t::ERROR;
    }
}

void ATCmd::okReceived(void)
{
    Trace(ZONE_INFO, "ATCMD: %s OK\n", mName.data());
    mSendResult.overwrite(true);
}

void ATCmd::errorReceived(void)
{
    Trace(ZONE_INFO, "ATCMD: %s ERROR\n", mName.data());
    mSendResult.overwrite(false);
}

AT::Return_t ATCmd::onResponseMatch(void)
{
    Trace(ZONE_INFO, "Hello from %s\n", mName.data());
    return Return_t::WAITING;
}

//------------------------ATCmdTX---------------------------------

AT::Return_t ATCmdTX::onResponseMatch(void)
{
    if (mSendFunction(mData, ATParser::defaultTimeout) != mData.length()) {
        Trace(ZONE_ERROR, "Couldn't send data\n");
        return Return_t::ERROR;
    }
    return Return_t::WAITING;
}

//------------------------ATCmdUSOST---------------------------------

AT::Return_t ATCmdUSOST::send(const size_t                    socket,
                              const std::string_view          ip,
                              const std::string_view          port,
                              const std::string_view          data,
                              const std::chrono::milliseconds timeout)
{
    if (data.length() == 0) {
        Trace(ZONE_WARNING, "Nodata %d\r\n", data.length());
        return AT::Return_t::FINISHED;
    }
    mData = data;
    const size_t reqLen = std::snprintf(
                                        mRequestBuffer.data(),
                                        mRequestBuffer.size(),
                                        "AT+USOST=%d,\"%s\",%s,%d\r",
                                        socket,
                                        ip.data(),
                                        port.data(),
                                        data.length());

    mRequest = std::string_view(mRequestBuffer.data(), reqLen);

    return ATCmd::send(mSendFunction, timeout);
}

//------------------------ATCmdUSOWR---------------------------------

AT::Return_t ATCmdUSOWR::send(const size_t                    socket,
                              const std::string_view          data,
                              const std::chrono::milliseconds timeout)
{
    if (data.length() == 0) {
        Trace(ZONE_WARNING, "Nodata %d\r\n", data.length());
        return AT::Return_t::FINISHED;
    }
    mData = data;
    const size_t reqLen = std::snprintf(
                                        mRequestBuffer.data(),
                                        mRequestBuffer.size(),
                                        "AT+USOWR=%d,%d\r",
                                        socket,
                                        data.length());

    mRequest = std::string_view(mRequestBuffer.data(), reqLen);

    return ATCmd::send(mSendFunction, timeout);
}

//------------------------ATCmdRXData---------------------------------

std::string_view ATCmdRXData::getData(void) const
{
    return mData;
}

AT::Return_t ATCmdRXData::getDataFromParser(const size_t bytesAvailable)
{
    const std::string_view datastring = mParser->getBytesFromInput(bytesAvailable + 2);
    if (datastring.length() < bytesAvailable) {
        Trace(ZONE_ERROR, "datastringLength %d %d\r\n", datastring.length(), bytesAvailable);
        return Return_t::ERROR;
    }

    auto first = datastring.find_first_of('"') + 1;
    auto last = datastring.find_last_of('"');

    mData = std::string_view(mDataBuffer.data(), last - first);

    if ((mData.length() == 0) || (mData.length() > mDataBuffer.size())) {
        Trace(ZONE_ERROR, "data length\r\n");
        return AT::Return_t::ERROR;
    }

    std::memcpy(mDataBuffer.data(), datastring.data() + first, mData.length());
    return AT::Return_t::FINISHED;
}

//------------------------ATCmdUSORF---------------------------------

AT::Return_t ATCmdUSORF::send(const size_t                    socket,
                              size_t                          bytesToRead,
                              const std::chrono::milliseconds timeout)
{
    if (bytesToRead > sizeof(mDataBuffer)) {
        Trace(ZONE_INFO, "More bytes available than readable\r\n");
        bytesToRead = sizeof(mDataBuffer);
    }
    const size_t reqLen = std::snprintf(
                                        mRequestBuffer.data(),
                                        mRequestBuffer.size(),
                                        "AT+USORF=%d,%d\r",
                                        socket, bytesToRead);

    mRequest = std::string_view(mRequestBuffer.data(), reqLen);
    return ATCmd::send(mSendFunction, timeout);
}

AT::Return_t ATCmdUSORF::onResponseMatch(void)
{
    if (mParser->getSocketFromInput(mSocket) != Return_t::FINISHED) {
        return Return_t::ERROR;
    }

    char termination = 0;
    const std::string_view ipstring = mParser->getInputUntilComma(&termination);
    if (ipstring.length() == 0) {
        return AT::Return_t::ERROR;
    }

    if (termination == ',') {
        // ---------------- IP ----------------------
        const auto first = ipstring.find_first_of('"') + 1;
        const auto last = ipstring.find_last_of('"');

        mIp = std::string_view(mIpBuffer.data(), last - first);

        if ((mIp.length() == 0) || (mIp.length() > mIpBuffer.size())) {
            return AT::Return_t::ERROR;
        }

        std::memcpy(mIpBuffer.data(), ipstring.data() + first, mIp.length());
        // ---------------- PORT ----------------------
        const std::string_view portstring = mParser->getInputUntilComma();
        if (portstring.length() == 0) {
            return AT::Return_t::ERROR;
        }

        mPort = std::string_view(mPortBuffer.data(), portstring.length());

        if ((mPort.length() == 0) || (mPort.length() > mPortBuffer.size())) {
            return AT::Return_t::ERROR;
        }

        std::memcpy(mPortBuffer.data(), portstring.data(), mData.length());
        // ---------------- BYTES AVAILABLE ----------------------
        size_t bytesAvailable = 0;
        if (mParser->getNumberFromInput(bytesAvailable) != Return_t::FINISHED) {
            return Return_t::ERROR;
        }
        // ---------------- DATA ----------------------
        if (getDataFromParser(bytesAvailable) != Return_t::FINISHED) {
            return Return_t::ERROR;
        }
        return Return_t::WAITING;
    } else if (termination == '\r') {
        size_t bytesAvailable = 0;
        if (mParser->strToNum(bytesAvailable, ipstring) != Return_t::FINISHED) {
            return Return_t::ERROR;
        }
        mUrcReceivedCallback(mSocket, bytesAvailable);
        return Return_t::WAITING;
    } else {
        Trace(ZONE_ERROR, "termination failure\r\n");
        return Return_t::ERROR;
    }
}

//------------------------ATCmdUSORD---------------------------------

AT::Return_t ATCmdUSORD::send(const size_t socket, size_t bytesToRead, const std::chrono::milliseconds timeout)
{
    if (bytesToRead > sizeof(mDataBuffer)) {
        Trace(ZONE_INFO, "More bytes available than readable\r\n");
        bytesToRead = sizeof(mDataBuffer);
    }
    const size_t reqLen = std::snprintf(
                                        mRequestBuffer.data(),
                                        mRequestBuffer.size(),
                                        "AT+USORD=%d,%d\r",
                                        socket, bytesToRead);

    mRequest = std::string_view(mRequestBuffer.data(), reqLen);
    return ATCmd::send(mSendFunction, timeout);
}

AT::Return_t ATCmdUSORD::onResponseMatch(void)
{
    if (mParser->getSocketFromInput(mSocket) != Return_t::FINISHED) {
        Trace(ZONE_ERROR, "socket\r\n");
        return Return_t::ERROR;
    }

    char termination = 0;
    size_t bytesAvailable = 0;
    if (mParser->getNumberFromInput(bytesAvailable, &termination) != Return_t::FINISHED) {
        Trace(ZONE_ERROR, "bytesAvailable\r\n");
        return Return_t::ERROR;
    }
    if (termination == ',') {
        if (getDataFromParser(bytesAvailable) != Return_t::FINISHED) {
            return Return_t::ERROR;
        }
        return Return_t::WAITING;
    } else if (termination == '\r') {
        mUrcReceivedCallback(mSocket, bytesAvailable);
        return Return_t::WAITING;
    } else {
        Trace(ZONE_ERROR, "termination failure\r\n");
        return Return_t::ERROR;
    }
}

//------------------------ATCmdUPSND---------------------------------

const std::string_view ATCmdUPSND::getData(void) const
{
    return mData;
}

size_t ATCmdUPSND::getParameter(void) const
{
    return mParameter;
}

size_t ATCmdUPSND::getSocket(void) const
{
    return mSocket;
}

AT::Return_t ATCmdUPSND::send(const size_t socket, const size_t parameter, const std::chrono::milliseconds timeout)
{
    const size_t reqLen = std::snprintf(
                                        mRequestBuffer.data(),
                                        mRequestBuffer.size(),
                                        "AT+UPSND=%d,%d\r",
                                        socket, parameter);

    mRequest = std::string_view(mRequestBuffer.data(), reqLen);
    return ATCmd::send(mSendFunction, timeout);
}

AT::Return_t ATCmdUPSND::onResponseMatch(void)
{
    if (mParser->getSocketFromInput(mSocket) != Return_t::FINISHED) {
        return Return_t::ERROR;
    }

    if (mParser->getNumberFromInput(mParameter) != Return_t::FINISHED) {
        return Return_t::ERROR;
    }

    const std::string_view datastring = mParser->getInputUntilComma();
    if ((datastring.length() == 0) || (datastring.length() > sizeof(mDataBuffer))) {
        return AT::Return_t::ERROR;
    }

    auto first = datastring.find_first_of('"') + 1;
    auto last = datastring.find_last_of('"');

    mData = std::string_view(mDataBuffer.data(), last - first);

    if ((mData.length() == 0) || (mData.length() > sizeof(mDataBuffer))) {
        return AT::Return_t::ERROR;
    }

    std::memcpy(mDataBuffer.data(), datastring.data() + first, mData.length());
    return Return_t::WAITING;
}

//------------------------ATCmdUSOCR---------------------------------

size_t ATCmdUSOCR::getSocket(void) const
{
    return mSocket;
}

AT::Return_t ATCmdUSOCR::send(const size_t protocol, const std::chrono::milliseconds timeout)
{
    const size_t reqLen = std::snprintf(
                                        mRequestBuffer.data(),
                                        mRequestBuffer.size(),
                                        "AT+USOCR=%d\r",
                                        protocol);

    mRequest = std::string_view(mRequestBuffer.data(), reqLen);
    return ATCmd::send(mSendFunction, timeout);
}

AT::Return_t ATCmdUSOCR::onResponseMatch(void)
{
    if (mParser->getSocketFromInput(mSocket) != Return_t::FINISHED) {
        return Return_t::ERROR;
    }
    return Return_t::WAITING;
}

//------------------------ATCmdUSOCO---------------------------------

AT::Return_t ATCmdUSOCO::send(const size_t                    socket,
                              const std::string_view          ip,
                              const std::string_view          port,
                              const std::chrono::milliseconds timeout)
{
    const size_t reqLen = std::snprintf(
                                        mRequestBuffer.data(),
                                        mRequestBuffer.size(),
                                        "AT+USOCO=%d,\"%s\",%s\r",
                                        socket,
                                        ip.data(),
                                        port.data());

    mRequest = std::string_view(mRequestBuffer.data(), reqLen);

    return ATCmd::send(mSendFunction, timeout);
}

//------------------------ATCmdUSOSO---------------------------------

AT::Return_t ATCmdUSOSO::send(const size_t                    socket,
                              const size_t                    level,
                              const size_t                    optName,
                              const size_t                    optVal,
                              const std::chrono::milliseconds timeout)
{
    const size_t reqLen = std::snprintf(
                                        mRequestBuffer.data(),
                                        mRequestBuffer.size(),
                                        "AT+USOSO=%d,%d,%d,%d\r",
                                        socket,
                                        level,
                                        optName,
                                        optVal);

    mRequest = std::string_view(mRequestBuffer.data(), reqLen);

    return ATCmd::send(mSendFunction, timeout);
}

//------------------------ATCmdUSOCTL---------------------------------

const std::string_view ATCmdUSOCTL::getData(void) const
{
    return mData;
}

size_t ATCmdUSOCTL::getParamId(void) const
{
    return mParamId;
}

size_t ATCmdUSOCTL::getSocket(void) const
{
    return mSocket;
}

AT::Return_t ATCmdUSOCTL::send(const size_t                    socket,
                               const size_t                    paramId,
                               const std::chrono::milliseconds timeout)
{
    const size_t reqLen = std::snprintf(
                                        mRequestBuffer.data(),
                                        mRequestBuffer.size(),
                                        "AT+USOCTL=%d,%d\r",
                                        socket,
                                        paramId
                                        );

    mRequest = std::string_view(mRequestBuffer.data(), reqLen);

    return ATCmd::send(mSendFunction, timeout);
}

AT::Return_t ATCmdUSOCTL::onResponseMatch(void)
{
    if (mParser->getSocketFromInput(mSocket) != Return_t::FINISHED) {
        return Return_t::ERROR;
    }

    if (mParser->getNumberFromInput(mParamId) != Return_t::FINISHED) {
        return Return_t::ERROR;
    }

    mData = mParser->getInputUntilComma();

    if ((mData.length() == 0) || (mData.length() > sizeof(mDataBuffer))) {
        return AT::Return_t::ERROR;
    }
    std::memcpy(mDataBuffer.data(), mData.data(), mData.length());
    return Return_t::WAITING;
}

//------------------------ATCmdURC---------------------------------

void ATCmdURC::okReceived(void)
{
    Trace(ZONE_INFO, "%s OK\n", mName.data());
    while (true) {}
}
void ATCmdURC::errorReceived(void)
{
    Trace(ZONE_INFO, "%s ERROR\n", mName.data());
    while (true) {}
}

AT::Return_t ATCmdURC::onResponseMatch(void)
{
    char termination = 0;
    size_t socket = 0;
    if (mParser->getSocketFromInput(socket, &termination) != Return_t::FINISHED) {
        return Return_t::ERROR;
    }

    if (termination == '\r') {
        mUrcReceivedCallback(socket, 0);
        return Return_t::FINISHED;
    }

    size_t parameter;
    if (mParser->getNumberFromInput(parameter) != Return_t::FINISHED) {
        return Return_t::ERROR;
    }

    mUrcReceivedCallback(socket, parameter);
    return Return_t::FINISHED;
}

//------------------------ATCmdOK---------------------------------

void ATCmdOK::okReceived(void)
{
    Trace(ZONE_INFO, "%s OK\n", mName.data());
    while (true) {}
}
void ATCmdOK::errorReceived(void)
{
    Trace(ZONE_INFO, "%s ERROR\n", mName.data());
    while (true) {}
}

AT::Return_t ATCmdOK::onResponseMatch(void)
{
    if (mParser->mWaitingCmd) {
        mParser->mWaitingCmd->okReceived();
        mParser->mWaitingCmd = nullptr;
        return Return_t::FINISHED;
    }
    return Return_t::ERROR;
}

//------------------------ATCmdERROR---------------------------------

void ATCmdERROR::okReceived(void)
{
    Trace(ZONE_INFO, "%s OK\n", mName.data());
    while (true) {}
}
void ATCmdERROR::errorReceived(void)
{
    Trace(ZONE_INFO, "%s ERROR\n", mName.data());
    while (true) {}
}

AT::Return_t ATCmdERROR::onResponseMatch(void)
{
    if (mParser->mWaitingCmd) {
        mParser->mWaitingCmd->errorReceived();
        mParser->mWaitingCmd = nullptr;
        return Return_t::FINISHED;
    }
    return Return_t::ERROR;
}

//------------------------ATParser---------------------------------

std::array<char, ATParser::BUFFERSIZE> ATParser::ReceiveBuffer;

void ATParser::reset(void)
{
    if (mWaitingCmd) {
        Trace(ZONE_INFO, "Toogle Error on waiting cmd\r\n");
        mWaitingCmd->errorReceived();
        mWaitingCmd = nullptr;
    }
}

void ATParser::triggerMatch(AT* match)
{
    switch (match->onResponseMatch()) {
    case AT::Return_t::WAITING:
        mWaitingCmd = match;
        break;

    case AT::Return_t::ERROR:
        Trace(ZONE_ERROR, "Error occured \r\n");
        break;

    case AT::Return_t::TRY_AGAIN:
        break;

    case AT::Return_t::FINISHED:
        Trace(ZONE_INFO, "ParserFinished %s \r\n", match->mName.data());
        break;
    }
}

bool ATParser::parse(std::chrono::milliseconds timeout)
{
    size_t currentPos = 0;
    std::array<AT*, MAXATCMDS> possibleResponses;
    std::array<AT*, MAXATCMDS> sievedResponses;

    auto resetPossibleResponses = [&] {
                                      currentPos = 0;
                                      possibleResponses.fill(nullptr);
                                      std::copy(mRegisteredATCommands.begin(),
                                                mRegisteredATCommands.end(),
                                                possibleResponses.begin());
                                  };

    Trace(ZONE_INFO, "Start Parser\r\n");

    resetPossibleResponses();

    while (1 == mReceive(reinterpret_cast<uint8_t*>(ReceiveBuffer.data() + currentPos++), 1, timeout)) {
        std::string_view currentData(ReceiveBuffer.data(), currentPos);
        //Trace(ZONE_VERBOSE, "parse: %s\n", std::string(currentData.data(), currentData.length()).c_str());

        {
            os::LockGuard<os::Mutex> lock(mWaitingCmdMutex);

            if (mWaitingCmd && (currentData == mWaitingCmd->mResponse)) {
                Trace(ZONE_INFO, "Waiting MATCH: %s\n", mWaitingCmd->mName.data());
                triggerMatch(mWaitingCmd);
                resetPossibleResponses();
            }

            sievedResponses.fill(nullptr);

            std::copy_if(possibleResponses.begin(),
                         possibleResponses.end(),
                         sievedResponses.begin(),
                         [&](AT* atcmd){
                return atcmd != nullptr && currentData ==
                atcmd->mResponse.substr(0, currentPos);
            });

            possibleResponses = sievedResponses;
            const size_t possibleResponsesCount = std::count_if(possibleResponses.begin(),
                                                                possibleResponses.end(),
                                                                [](AT* cmd){
                return cmd != nullptr;
            });

            if (possibleResponsesCount > 1) { continue; }

            if (possibleResponsesCount == 1) {
                const auto matchIt = std::find_if(possibleResponses.begin(),
                                                  possibleResponses.end(), [](AT* cmd){
                    return cmd != nullptr;
                });
                if (matchIt == possibleResponses.end()) {
                    continue;
                }
                if (currentData != (*matchIt)->mResponse) {
                    continue;
                }
                Trace(ZONE_INFO, "MATCH: %s\n", (*matchIt)->mName.data());
                triggerMatch(*matchIt);
            }
        }
        resetPossibleResponses();
    }

    Trace(ZONE_ERROR, "Parser Timeout\r\n");
    reset();
    return false;
}

void ATParser::registerAtCommand(AT* cmd)
{
    if (mRegisteredATCommands.size() < MAXATCMDS) {
        cmd->mParser = this;
        mRegisteredATCommands.push_back(cmd);
    } else {
        Trace(ZONE_ERROR, "Can't register more AT commands");
    }
}

std::string_view ATParser::getLineFromInput(std::chrono::milliseconds timeout) const
{
    uint8_t data;
    size_t currentPos = 0;

    while (mReceive(&data, 1, timeout)) {
        ReceiveBuffer[currentPos++] = data;

        const bool terminationFound = (data == '\r') || (data == '\n') || (data == 0);

        if (terminationFound) {
            return std::string_view(ReceiveBuffer.data(), currentPos);
        }

        if (currentPos >= BUFFERSIZE) {
            Trace(ZONE_ERROR, "ReceiveBufferOverflow\r\n");
            return "";
        }
    }
    Trace(ZONE_ERROR, "Timeout\r\n");
    return "";
}

std::string_view ATParser::getInputUntilComma(char* const termination, std::chrono::milliseconds timeout) const
{
    uint8_t data;
    size_t currentPos = 0;

    while (mReceive(&data, 1, timeout)) {
        const bool terminationFound = (data == ',') || (data == '\r');

        if (terminationFound || isalpha(data)) {
            if (termination != nullptr) {
                *termination = data;
            }
            return std::string_view(ReceiveBuffer.data(), currentPos);
        }
        ReceiveBuffer[currentPos++] = data;

        if (currentPos >= BUFFERSIZE) {
            Trace(ZONE_ERROR, "ReceiveBufferOverflow\r\n");
            return "";
        }
    }
    Trace(ZONE_ERROR, "Timeout\r\n");
    return "";
}

std::string_view ATParser::getBytesFromInput(size_t numberOfBytes, std::chrono::milliseconds timeout) const
{
    uint8_t data;
    size_t currentPos = 0;

    if (numberOfBytes >= BUFFERSIZE) {
        return "";
    }

    while (mReceive(&data, 1, timeout)) {
        ReceiveBuffer[currentPos++] = data;

        const bool terminationFound = currentPos >= numberOfBytes;
        if (terminationFound) {
            return std::string_view(ReceiveBuffer.data(), currentPos);
        }
    }
    Trace(ZONE_ERROR, "Timeout\r\n");
    return "";
}

AT::Return_t ATParser::getSocketFromInput(size_t& socket, char* const termination,
                                          std::chrono::milliseconds timeout) const
{
    if (getNumberFromInput(socket, termination, timeout) != AT::Return_t::FINISHED) {
        return AT::Return_t::ERROR;
    }

    if (socket > 6) {
        return AT::Return_t::ERROR;
    }
    return AT::Return_t::FINISHED;
}

AT::Return_t ATParser::getNumberFromInput(size_t&                   number,
                                          char* const               termination,
                                          std::chrono::milliseconds timeout) const
{
    const std::string_view numstring = getInputUntilComma(termination, timeout);
    return strToNum(number, numstring);
}

AT::Return_t ATParser::strToNum(size_t&                number,
                                const std::string_view numstring) const
{
    if ((numstring.length() == 0) || (numstring.length() > 8)) {
        return AT::Return_t::ERROR;
    }
    std::array<char, 8> tempBuffer;
    tempBuffer.fill(0);
    std::memcpy(tempBuffer.data(), numstring.data(),
                std::min(numstring.length(), tempBuffer.size()));

    number = std::strtoul(tempBuffer.data(), nullptr, 10);
    return AT::Return_t::FINISHED;
}
