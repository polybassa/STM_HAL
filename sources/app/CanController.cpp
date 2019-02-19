// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "CanController.h"
#include "trace.h"
#include <algorithm>
#include <cstring>

using app::CanController;

static const int __attribute__((unused)) g_DebugZones = 0; //ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

os::StreamBuffer<char, CanController::BUFFERSIZE> CanController::ReceiveBuffer;

extern "C" char _binary_start;
extern "C" char _binary_end;

void CanController::CanControllerInterruptHandler(uint8_t data)
{
    ReceiveBuffer.sendFromISR(data);
}

CanController::CanController(const hal::UsartWithDma& interface,
                             const hal::Gpio&         supplyPin,
                             const hal::Gpio&         usartTxPin) :
    os::DeepSleepModule(),
    mTask("CanTask",
          CanController::STACKSIZE,
          os::Task::Priority::HIGH,
          [&](const bool& join)
{
    taskFunction(join);
}),
    mInterface(interface),
    mCanSupplyVoltage(supplyPin),
    mUsartTxPin(usartTxPin)
{
    mInterface.mUsart.enableNonBlockingReceive(CanControllerInterruptHandler);
}

void CanController::enterDeepSleep(void)
{
    off();
    mTask.join();
}

void CanController::exitDeepSleep(void)
{
    mTask.start();
    on();
}

void CanController::taskFunction(const bool& join)
{
    do {
        if (mReceiveCallback) {
            if (ReceiveBuffer.bytesAvailable()) {
                const size_t length = ReceiveBuffer.receive(
                                                            mTempReceiveCallbackBuffer.data(),
                                                            mTempReceiveCallbackBuffer.size(),
                                                            std::chrono::milliseconds(100));
                mReceiveCallback(std::string_view(mTempReceiveCallbackBuffer.data(), length));
                continue;
            }
        }

        if (mIsPerformingFirmwareUpdate) {
            Trace(ZONE_INFO, "START FLASH... \r\n");
            flashSecCoFirmware();
        } else {
            os::ThisTask::sleep(std::chrono::milliseconds(10));
        }
    } while (!join);
}

void CanController::flashSecCoFirmware(void)
{
    mWasFirmwareUpdateSuccessful = false;
    mWasFirmwareUpdateSuccessful = flash(std::string_view(
                                                          reinterpret_cast<char*>(&_binary_start),
                                                          reinterpret_cast<char*>(&_binary_end) -
                                                          reinterpret_cast<char*>(&_binary_start)),
                                         0x8000000);

    mIsPerformingFirmwareUpdate = false;
}

bool CanController::connectToBootloader(void)
{
    ReceiveBuffer.reset();
    Trace(ZONE_INFO, "Connecting to bootloader... ");
    mInterface.send("\x7f");
    return receiveResponseFromBootloader();
}

bool CanController::sendCommandToBootloader(std::string_view cmd)
{
    sendToBootloaderWithChecksum(cmd, 0xff);
    return receiveResponseFromBootloader();
}

size_t CanController::sendToBootloaderWithChecksum(std::string_view data, uint8_t initvalue)
{
    uint8_t sum = initvalue;
    std::for_each(
                  data.begin(),
                  data.end(),
                  [&sum](const auto& d){
        sum ^= d;
    });

    ReceiveBuffer.reset();
    auto ret = mInterface.send(data, 100);
    mInterface.send(&sum, 1);
    return ret + 1;
}

bool CanController::receiveResponseFromBootloader(void)
{
    static constexpr const uint8_t ACK = 0x79;

    char uret;
    if (!ReceiveBuffer.receive(uret, std::chrono::milliseconds(100))) {
        Trace(ZONE_INFO, "Nothing received... \r\n");
        return false;
    }
    if (uret != ACK) { return false; }
    Trace(ZONE_INFO, "ACK.\r\n");
    return true;
}

uint32_t CanController::swap(const uint32_t num)
{
    return ((num >> 24) & 0xff) | // move byte 3 to byte 0
           ((num << 8) & 0xff0000) | // move byte 1 to byte 2
           ((num >> 8) & 0xff00) | // move byte 2 to byte 1
           ((num << 24) & 0xff000000); // byte 0 to byte 3
}

bool CanController::eraseChip(void)
{
    resetToBootloader();

    if (!connectToBootloader()) {
        return false;
    }

    Trace(ZONE_INFO, "Sending erase... ");
    auto ret = sendCommandToBootloader("\x43");
    if (ret) {
        Trace(ZONE_INFO, "Sending global erase... ");
        if (!sendCommandToBootloader("\xFF")) { return false; }
    } else {
        Trace(ZONE_INFO, "Sending readout unprotect command... ");
        if (!sendCommandToBootloader("\x92")) { return false; }
        if (!receiveResponseFromBootloader()) { return false; }
    }
    return true;
}

bool CanController::sendGoCommand(const uint32_t goAddress)
{
    Trace(ZONE_INFO, "Sending go command... ");

    if (!sendCommandToBootloader("\x21")) {
        return false;
    }

    Trace(ZONE_INFO, "Sending go address... ");
    auto dst_be = swap(goAddress);
    sendToBootloaderWithChecksum(std::string_view(reinterpret_cast<const char*>(&dst_be), 4));
    if (!receiveResponseFromBootloader()) { return false; }
    return true;
}

bool CanController::flash(std::string_view data, const size_t address)
{
    Trace(ZONE_INFO, "Flash 0x%x bytes. \r\n", data.length());

    if (!eraseChip()) {
        return false;
    }

    resetToBootloader();

    if (!connectToBootloader()) {
        return false;
    }

    uint8_t N = 0;
    size_t dst = address;
    size_t src_idx = 0;
    size_t len = data.length();

    while (len > 0) {
        Trace(ZONE_INFO, "Sending write command... ");
        if (!sendCommandToBootloader("\x31")) { return false; }

        Trace(ZONE_INFO, "Sending write address... ");
        auto dst_be = swap(dst);
        sendToBootloaderWithChecksum(std::string_view(reinterpret_cast<const char*>(&dst_be), 4));
        if (!receiveResponseFromBootloader()) { return false; }

        Trace(ZONE_INFO, "Sending data to write... ");
        N = len >= 256 ? 255 : len - 1;
        mInterface.send(&N, 1, 100);
        sendToBootloaderWithChecksum(std::string_view(&data.data()[src_idx], N + 1), N);
        if (!receiveResponseFromBootloader()) { return false; }

        len -= N + 1;
        dst += N + 1;
        src_idx += N + 1;
    }

    return sendGoCommand(address);
}

void CanController::resetToBootloader(void)
{
    Trace(ZONE_INFO, "Rebooting target. \r\n");
    off();
    mUsartTxPin.configureAsOutput();
    mUsartTxPin = false;
    os::ThisTask::sleep(std::chrono::milliseconds(100));
    on();
    os::ThisTask::sleep(std::chrono::milliseconds(25));
    mUsartTxPin.restoreDefaultConfiguration();
}

void CanController::on(void)
{
    mCanSupplyVoltage = true;
}

void CanController::off(void)
{
    mCanSupplyVoltage = false;
}

void CanController::triggerFirmwareUpdate(void)
{
    if (!mIsPerformingFirmwareUpdate) {
        mIsPerformingFirmwareUpdate = true;
    }
}

bool CanController::isPerformingFirmwareUpdate(void) const
{
    return mIsPerformingFirmwareUpdate;
}

bool CanController::wasFirmwareUpdateSuccessful(void) const
{
    return mWasFirmwareUpdateSuccessful;
}

size_t CanController::send(std::string_view message, const uint32_t ticksToWait)
{
    if (!mIsPerformingFirmwareUpdate) {
        return mInterface.send(message, ticksToWait);
    }
    return 0;
}

//TODO change ticksToWait to chrono
size_t CanController::receive(uint8_t* message, size_t length, uint32_t ticksToWait)
{
    if (mIsPerformingFirmwareUpdate) {
        Trace(ZONE_ERROR, "CanController is performing an update. Can't receive at the moment!\r\n ");
    } else if (mReceiveCallback != nullptr) {
        Trace(ZONE_ERROR, "A receive callback is registered. Use that one to get data!\r\n ");
    } else {
        return ReceiveBuffer.receive(reinterpret_cast<char*>(message), length, std::chrono::milliseconds(ticksToWait));
    }
    return 0;
}

void CanController::registerReceiveCallback(std::function<void(std::string_view)> f)
{
    mReceiveCallback = f;
}
void CanController::unregisterReceiveCallback(void)
{
    mReceiveCallback = nullptr;
}
