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

#include "CanController.h"
#include "trace.h"
#include <string>
#include <algorithm>
#include <cstring>

using app::CanController;

static const int __attribute__((unused)) g_DebugZones = 0; // ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

os::StreamBuffer<uint8_t, CanController::BUFFERSIZE> CanController::ReceiveBuffer;
os::Semaphore CanController::FrameAvailable;

extern "C" char _binary_start;
extern "C" char _binary_end;

void CanController::CanControllerInterruptHandler(uint8_t data)
{
    ReceiveBuffer.sendFromISR(data);

    if (data == '\r') {
        FrameAvailable.giveFromISR();
    }
}

CanController::CanController(const hal::UsartWithDma& interface,
                             const hal::Gpio&         supplyPin,
                             const hal::Gpio&         usartTxPin) :
    os::DeepSleepModule(),
    mTask("CanTask",
          CanController::STACKSIZE,
          os::Task::Priority::HIGH,
          [this](const bool& join)
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
    std::string tmpString(BUFFERSIZE, '\x00');

    do {
        if (mReceiveCallback) {
            if (FrameAvailable.take(std::chrono::milliseconds(100))) {
                ReceiveBuffer.receive(tmpString.data(), ReceiveBuffer.bytesAvailable(), 100);
                mReceiveCallback(tmpString);
            }
        }

        if (mIsPerformingFirmwareUpdate) {
            Trace(ZONE_INFO, "START FLASH... \r\n");

            flashSecCoFirmware();
        } else {
            os::ThisTask::sleep(std::chrono::milliseconds(500));
        }
    } while (!join);
}

void CanController::flashSecCoFirmware(void)
{
    mWasFirmwareUpdateSuccessful = false;

    auto ret = flash(std::string_view(reinterpret_cast<char*>(&_binary_start),
                                      reinterpret_cast<char*>(&_binary_end) - reinterpret_cast<char*>(&_binary_start)),
                     0x8000000);

    mWasFirmwareUpdateSuccessful = ret;
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
    std::for_each(data.begin(), data.end(), [&sum](const auto & d){
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

    uint8_t uret;
    if (!ReceiveBuffer.receive(uret, 100)) {
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
};

bool CanController::eraseChip(void)
{
    resetToBootloader();

    if (!connectToBootloader()) {
        return false;
    }

    Trace(ZONE_INFO, "Sending erase... ");
    auto ret = sendCommandToBootloader("\x43");
    if (ret) {
        Trace(ZONE_INFO, "ACK.\r\n");
        Trace(ZONE_INFO, "Sending global erase... ");
        if (!sendCommandToBootloader("\xFF")) { return false; }
        Trace(ZONE_INFO, "ACK.\r\n");
    } else {
        Trace(ZONE_INFO, "NACK.\r\n");
        Trace(ZONE_INFO, "Sending readout unprotect command... ");
        if (!sendCommandToBootloader("\x92")) { return false; }
        Trace(ZONE_INFO, "ACK... ");
        if (!receiveResponseFromBootloader()) { return false; }
        Trace(ZONE_INFO, "ACK.\r\n");
    }
    return true;
}

bool CanController::sendGoCommand(const uint32_t goAddress)
{
    Trace(ZONE_INFO, "Sending go command... ");

    if (!sendCommandToBootloader("\x21")) { return false; }

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

    uint8_t buf[257];
    size_t N = 0;
    size_t dst = address;
    size_t src_idx = 0;

    resetToBootloader();

    if (!connectToBootloader()) {
        return false;
    }

    size_t len = data.length();

    while (len > 0) {
        Trace(ZONE_INFO, "Sending write command... ");
        if (!sendCommandToBootloader("\x31")) { return false; }

        Trace(ZONE_INFO, "Sending write address... ");
        auto dst_be = swap(dst);
        sendToBootloaderWithChecksum(std::string_view(reinterpret_cast<const char*>(&dst_be), 4));
        if (!receiveResponseFromBootloader()) { return false; }

        Trace(ZONE_INFO, "Sending data to write... ");
        if (len >= 256) {
            N = 255;
        } else {
            N = len - 1;
        }
        buf[0] = N;
        mInterface.send(buf, 1, 100);
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
    mIsPerformingFirmwareUpdate = true;
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

size_t CanController::receive(uint8_t* message, size_t length, uint32_t ticksToWait)
{
    if (!mIsPerformingFirmwareUpdate) {
        if (FrameAvailable.take(std::chrono::milliseconds(ticksToWait))) {
            return ReceiveBuffer.receive(reinterpret_cast<char*>(message), length, ticksToWait);
        }
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
