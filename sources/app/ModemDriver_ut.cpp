/* Copyright (C) 2016  Nils Weiss
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

#include <cmath>
#include <thread>
#include <iostream>
#include <cstring>
#include "unittest.h"
#include "ModemDriver.h"

#define NUM_TEST_LOOPS 255

//--------------------------BUFFERS--------------------------
uint32_t g_currentTickCount;
bool g_taskJoined, g_taskStarted;
std::thread::id g_masterThreadId;
static constexpr size_t MEMORYSIZE = 255;
uint8_t g_masterRxMemroy[MEMORYSIZE];
bool g_comError = false;

//--------------------------MOCKING--------------------------
constexpr const std::array<const hal::Gpio, hal::Gpio::__ENUM__SIZE + 1> hal::Factory<hal::Gpio>::Container;
constexpr const std::array<const hal::Usart, hal::Usart::__ENUM__SIZE + 1> hal::Factory<hal::Usart>::Container;
constexpr const std::array<const hal::Dma, hal::Dma::__ENUM__SIZE + 1> hal::Factory<hal::Dma>::Container;
constexpr const std::array<const hal::UsartWithDma,
                           hal::Usart::__ENUM__SIZE> hal::Factory<hal::UsartWithDma>::Container;

void os::TaskInterruptable::join(void)
{
    g_taskJoined = true;
}

void os::TaskInterruptable::start(void)
{
    g_taskStarted = true;
}

os::TaskInterruptable::TaskInterruptable(char const* name, unsigned short stack, os::Task::Priority prio,
                                         std::function<void(bool const&)> func) :
    Task(name, stack, prio, func) {}

os::TaskInterruptable::~TaskInterruptable(void) {}

void os::TaskInterruptable::taskFunction(void) {}

os::Task::Task(char const* name, unsigned short stack, os::Task::Priority prio,
               std::function<void(bool const&)> func) {}

os::Task::~Task(void) {}

void os::Task::taskFunction(void) {}

void os::ThisTask::sleep(const std::chrono::milliseconds ms) {}

uint32_t os::Task::getTickCount(void)
{
    return g_currentTickCount;
}

void os::ThisTask::yield(void)
{}

size_t hal::UsartWithDma::send(uint8_t const* const data, const size_t length, const uint32_t ticksToWait) const
{
    std::memcpy(g_masterRxMemroy, data, length);
    return g_comError ? 0 : length;
}

size_t hal::UsartWithDma::receive(uint8_t* const data, const size_t length, const uint32_t ticksToWait) const
{
    std::memcpy(data, g_masterRxMemroy, length);
    return g_comError ? 0 : length;
}

void hal::Usart::enableNonBlockingReceive(std::function<void(uint8_t)> callback) const
{}

size_t hal::UsartWithDma::send(std::string_view str, const uint32_t ticksToWait) const
{
    return send(reinterpret_cast<uint8_t const* const>(str.data()), str.length(), ticksToWait);
}

void hal::Gpio::operator=(const bool& state) const
{}

void os::ThisTask::enterCriticalSection() {}

void os::ThisTask::exitCriticalSection() {}

StreamBufferHandle_t xStreamBufferGenericCreate(size_t     xBufferSizeBytes,
                                                size_t     xTriggerLevelBytes,
                                                BaseType_t xIsMessageBuffer)
{
    return 0;
}

size_t xStreamBufferSendFromISR(StreamBufferHandle_t xStreamBuffer,
                                const void*          pvTxData,
                                size_t               xDataLengthBytes,
                                BaseType_t* const    pxHigherPriorityTaskWoken)
{ return 1; }

size_t xStreamBufferReceive(StreamBufferHandle_t xStreamBuffer,
                            void*                pvRxData,
                            size_t               xBufferLengthBytes,
                            TickType_t           xTicksToWait)
{ return 1; }

void vStreamBufferDelete(StreamBufferHandle_t xStreamBuffer) {}

QueueHandle_t xQueueGenericCreate(const UBaseType_t uxQueueLength,
                                  const UBaseType_t uxItemSize,
                                  const uint8_t     ucQueueType)
{
    return 0;
}

void vQueueDelete(QueueHandle_t xQueue) {}

BaseType_t xQueueGenericSend(QueueHandle_t     xQueue,
                             const void* const pvItemToQueue,
                             TickType_t        xTicksToWait,
                             const BaseType_t  xCopyPosition)
{
    return 0;
}

BaseType_t xQueueSemaphoreTake(QueueHandle_t xQueue, TickType_t xTicksToWait)
{
    return 0;
}

BaseType_t xQueueGenericReceive(QueueHandle_t    xQueue,
                                void* const      pvBuffer,
                                TickType_t       xTicksToWait,
                                const BaseType_t xJustPeeking)
{
    return 1;
}

BaseType_t xQueueGiveFromISR(QueueHandle_t xQueue, BaseType_t* const pxHigherPriorityTaskWoken)
{
    return 1;
}

BaseType_t xQueueReceiveFromISR(QueueHandle_t xQueue, void* const pvBuffer, BaseType_t* const pxHigherPriorityTaskWoken)
{
    return 1;
}
//-------------------------TESTCASES-------------------------

int ut_DeepSleep(void)
{
    TestCaseBegin();

    g_currentTickCount = 0;
    g_comError = false;
    memset(g_masterRxMemroy, 0, MEMORYSIZE);

    CHECK(false == g_taskJoined);
    CHECK(false == g_taskStarted);

    app::ModemDriver driver(hal::Factory<hal::UsartWithDma>::get<hal::Usart::MODEM_COM>(),
                            hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_1>(),
                            hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_2>(),
                            hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_3>());

    os::DeepSleepController::enterGlobalDeepSleep();

    CHECK(true == g_taskJoined);

    os::DeepSleepController::exitGlobalDeepSleep();

    CHECK(true == g_taskStarted);
    TestCaseEnd();
}

int ut_SplitDataString(void)
{
    TestCaseBegin();

    app::ModemDriver driver(hal::Factory<hal::UsartWithDma>::get<hal::Usart::MODEM_COM>(),
                            hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_1>(),
                            hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_2>(),
                            hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_3>());

    app::ModemDriverTester tester(driver);

    auto ret = tester.splitDataString("+USORF: SOCKET_IDX,\"IP_ADDRESS\",PORT,DATA_LEN,\"DATA\"");

    CHECK(ret.size() == 5);

    auto socket = ret[0];
    auto ip = ret[1];
    auto port = ret[2];
    auto len = ret[3];
    auto data = ret[4];

    TestCaseEnd();
}

int main(int argc, const char* argv[])
{
    UnitTestMainBegin();
    RunTest(true, ut_DeepSleep);
    RunTest(true, ut_SplitDataString);

    UnitTestMainEnd();
}
