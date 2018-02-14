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

using app::CanController;

static const int __attribute__((unused)) g_DebugZones = 0; //ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

os::StreamBuffer<uint8_t, CanController::BUFFERSIZE> CanController::ReceiveBuffer;
os::Semaphore CanController::FrameAvailable;

void CanController::CanControllerInterruptHandler(uint8_t data)
{
    ReceiveBuffer.sendFromISR(data);

    if (data == '\r') {
        FrameAvailable.giveFromISR();
    }
}

CanController::CanController(const hal::UsartWithDma& interface,
                             const hal::Gpio&         supplyPin) :
    os::DeepSleepModule(),
    mTask("CanTask",
          CanController::STACKSIZE,
          os::Task::Priority::HIGH,
          [this](const bool& join)
          {
              taskFunction(join);
          }),
    mInterface(interface),
    mCanSupplyVoltage(supplyPin)
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
        } else {
            os::ThisTask::sleep(std::chrono::milliseconds(500));
        }
    } while (!join);
}

void CanController::on(void)
{
    mCanSupplyVoltage = true;
}

void CanController::off(void)
{
    mCanSupplyVoltage = false;
}

size_t CanController::send(std::string_view message, const uint32_t ticksToWait)
{
    return mInterface.send(message, ticksToWait);
}

size_t CanController::receive(uint8_t* message, size_t length, uint32_t ticksToWait)
{
    if (FrameAvailable.take(std::chrono::milliseconds(ticksToWait))) {
        return ReceiveBuffer.receive(reinterpret_cast<char*>(message), length, ticksToWait);
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
