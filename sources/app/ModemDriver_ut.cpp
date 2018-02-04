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

static constexpr size_t MEMORYSIZE = 1024;
uint8_t g_usartMemory[MEMORYSIZE];
bool g_usartError = false;
bool g_usartNonBlockingEnabled = false;
std::function<void(uint8_t)> g_usartCallback;

bool g_gpioState = false;

static constexpr size_t STREAMBUFFERS = 2;
std::array<std::array<uint8_t, MEMORYSIZE>, STREAMBUFFERS> g_streamBufferMemorys;
std::array<std::array<uint8_t, MEMORYSIZE>::iterator, STREAMBUFFERS> g_streamBufferSendIterators;
std::array<std::array<uint8_t, MEMORYSIZE>::iterator, STREAMBUFFERS> g_streamBufferReceiveIterators;
int g_counterOfReturnedStreamBufferHandles = -1;

bool g_semaphoreGiven = false;

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
    std::memcpy(g_usartMemory, data, length);
    return g_usartError ? 0 : length;
}

size_t hal::UsartWithDma::receive(uint8_t* const data, const size_t length, const uint32_t ticksToWait) const
{
    std::memcpy(data, g_usartMemory, length);
    return g_usartError ? 0 : length;
}

void hal::Usart::enableNonBlockingReceive(std::function<void(uint8_t)> callback) const
{
    g_usartNonBlockingEnabled = true;
    g_usartCallback = callback;
}

size_t hal::UsartWithDma::send(std::string_view str, const uint32_t ticksToWait) const
{
    return send(reinterpret_cast<uint8_t const* const>(str.data()), str.length(), ticksToWait);
}

void hal::Gpio::operator=(const bool& state) const
{
    g_gpioState = state;
}

void os::ThisTask::enterCriticalSection() {}

void os::ThisTask::exitCriticalSection() {}

StreamBufferHandle_t xStreamBufferGenericCreate(size_t     xBufferSizeBytes,
                                                size_t     xTriggerLevelBytes,
                                                BaseType_t xIsMessageBuffer)
{
    g_counterOfReturnedStreamBufferHandles++;
    g_streamBufferReceiveIterators[g_counterOfReturnedStreamBufferHandles] =
        g_streamBufferMemorys[g_counterOfReturnedStreamBufferHandles].begin();
    g_streamBufferSendIterators[g_counterOfReturnedStreamBufferHandles] =
        g_streamBufferMemorys[g_counterOfReturnedStreamBufferHandles].begin();

    return reinterpret_cast<void*>(g_counterOfReturnedStreamBufferHandles);
}

size_t xStreamBufferSendFromISR(StreamBufferHandle_t xStreamBuffer,
                                const void*          pvTxData,
                                size_t               xDataLengthBytes,
                                BaseType_t* const    pxHigherPriorityTaskWoken)
{
    auto& it = g_streamBufferSendIterators[reinterpret_cast<size_t>(xStreamBuffer)];
    size_t i;
    auto p = reinterpret_cast<const uint8_t*>(pvTxData);

    for (i = 0; i < xDataLengthBytes; i++) {
        *it++ = *p++;
    }
    return i;
}

size_t xStreamBufferReceive(StreamBufferHandle_t xStreamBuffer,
                            void*                pvRxData,
                            size_t               xBufferLengthBytes,
                            TickType_t           xTicksToWait)
{
    auto& it = g_streamBufferReceiveIterators[reinterpret_cast<size_t>(xStreamBuffer)];
    auto& it_end = g_streamBufferSendIterators[reinterpret_cast<size_t>(xStreamBuffer)];

    size_t i;
    uint8_t* p = reinterpret_cast<uint8_t*>(pvRxData);

    for (i = 0; i < xBufferLengthBytes && it != it_end; i++) {
        *p++ = *it++;
    }
    return i;
}

void vStreamBufferDelete(StreamBufferHandle_t xStreamBuffer) {}

size_t xStreamBufferBytesAvailable(StreamBufferHandle_t xStreamBuffer)
{
    auto it = g_streamBufferReceiveIterators[reinterpret_cast<size_t>(xStreamBuffer)];
    auto it_end = g_streamBufferSendIterators[reinterpret_cast<size_t>(xStreamBuffer)];
    size_t i;
    for (i = 0; it != it_end; i++) {
        it++;
    }
    return i;
}

BaseType_t xStreamBufferIsEmpty(StreamBufferHandle_t xStreamBuffer)
{
    auto it = g_streamBufferReceiveIterators[reinterpret_cast<size_t>(xStreamBuffer)];
    auto it_end = g_streamBufferSendIterators[reinterpret_cast<size_t>(xStreamBuffer)];
    return it == it_end;
}

BaseType_t xStreamBufferReset(StreamBufferHandle_t xStreamBuffer)
{
    g_streamBufferReceiveIterators[g_counterOfReturnedStreamBufferHandles] =
        g_streamBufferMemorys[g_counterOfReturnedStreamBufferHandles].begin();
    g_streamBufferSendIterators[g_counterOfReturnedStreamBufferHandles] =
        g_streamBufferMemorys[g_counterOfReturnedStreamBufferHandles].begin();
    return pdTRUE;
}

QueueHandle_t xQueueGenericCreate(const UBaseType_t uxQueueLength,
                                  const UBaseType_t uxItemSize,
                                  const uint8_t     ucQueueType)
{
    return reinterpret_cast<void*>(0xdeadbeef);
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
    if (g_semaphoreGiven) {
        g_semaphoreGiven = false;
        return true;
    }
    return false;
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
    g_semaphoreGiven = true;
    return g_semaphoreGiven;
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
    g_usartError = false;
    memset(g_usartMemory, 0, MEMORYSIZE);

    CHECK(false == g_taskJoined);
    CHECK(false == g_taskStarted);

    app::ModemDriver driver(hal::Factory<hal::UsartWithDma>::get<hal::Usart::MODEM_COM>(),
                            hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_RESET>(),
                            hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_POWER>(),
                            hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_SUPPLY>());

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
                            hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_RESET>(),
                            hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_POWER>(),
                            hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_SUPPLY>());

    app::ModemDriverTester tester(driver);
    {
        const auto ret = tester.splitDataString("+USORF: 18,\"108.217.247.74\",220,15,\"0123456789qwert\"");

        CHECK(ret.size() == 6);
        auto cmd = ret[0];
        CHECK(cmd == "+USORF");
        auto socket = ret[1];
        CHECK(socket == " 18");
        auto ip = ret[2];
        CHECK(ip == "\"108.217.247.74\"");
        auto port = ret[3];
        CHECK(port == "220");
        auto len = ret[4];
        CHECK(len == "15");
        auto data = ret[5];
        CHECK(data == "\"0123456789qwert\"");
    }
    {
        auto ret = tester.splitDataString("+UUSORF: 18,15");

        CHECK(ret.size() == 3);
        auto cmd = ret[0];
        CHECK(cmd == "+UUSORF");
        auto socket = ret[1];
        CHECK(socket == " 18");
        auto len = ret[2];
        CHECK(len == "15");
    }
    {
        auto ret = tester.splitDataString("+USORF: 18,\"108.217.247.74\",220,15,\"\x30\x31\x32\x00\"");

        CHECK(ret.size() == 6);
        auto cmd = ret[0];
        CHECK(cmd == "+USORF");
        auto socket = ret[1];
        CHECK(socket == " 18");
        auto ip = ret[2];
        CHECK(ip == "\"108.217.247.74\"");
        auto port = ret[3];
        CHECK(port == "220");
        auto len = ret[4];
        CHECK(len == "15");
        auto data = ret[5];
        CHECK(data == "\"012\x00\"");
    }
    TestCaseEnd();
}

int ut_SendRecvUSORF(void)
{
    TestCaseBegin();

    g_counterOfReturnedStreamBufferHandles = -1;
    std::string teststring = "+USORF: 18,\"108.217.247.74\",220,15,\"0123456789qwert\"\r";
    g_semaphoreGiven = false;

    app::ModemDriver driver(hal::Factory<hal::UsartWithDma>::get<hal::Usart::MODEM_COM>(),
                            hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_RESET>(),
                            hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_POWER>(),
                            hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_SUPPLY>());

    for (auto c : teststring) {
        app::ModemDriver::ModemDriverInterruptHandler(static_cast<uint8_t>(c));
    }

    app::ModemDriverTester tester(driver);

    std::string sendString = "USOST\r";

    auto x = tester.modemSendRecv(sendString);

    CHECK(x == -1); // got Timeout because OK is missing
    CHECK(tester.getDataString() == "0123456789qwert");
    CHECK_MEMCMP(g_usartMemory, sendString.c_str(), sendString.length());

    for (auto c : "OK\r") {
        app::ModemDriver::ModemDriverInterruptHandler(static_cast<uint8_t>(c));
    }
    sendString = "TESTSTRING\r";
    x = tester.modemSendRecv(sendString);
    CHECK_MEMCMP(g_usartMemory, sendString.c_str(), sendString.length());

    CHECK(x == 0); // got Timeout because OK is missing

    TestCaseEnd();
}

int ut_SendRecvERROR(void)
{
    TestCaseBegin();

    g_counterOfReturnedStreamBufferHandles = -1;
    std::string teststring = "ERROR\r";
    g_semaphoreGiven = false;

    app::ModemDriver driver(hal::Factory<hal::UsartWithDma>::get<hal::Usart::MODEM_COM>(),
                            hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_RESET>(),
                            hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_POWER>(),
                            hal::Factory<hal::Gpio>::get<hal::Gpio::MODEM_SUPPLY>());

    for (auto c : teststring) {
        app::ModemDriver::ModemDriverInterruptHandler(static_cast<uint8_t>(c));
    }

    app::ModemDriverTester tester(driver);

    auto x = tester.modemSendRecv("USOST\r");

    CHECK(x == 1); // got Timeout because OK is missing

    for (auto c : "OK\r") {
        app::ModemDriver::ModemDriverInterruptHandler(static_cast<uint8_t>(c));
    }
    x = tester.modemSendRecv("USOST\r");
    CHECK(x == 0); // got Timeout because OK is missing

    TestCaseEnd();
}

int main(int argc, const char* argv[])
{
    UnitTestMainBegin();
    RunTest(true, ut_DeepSleep);
    RunTest(true, ut_SplitDataString);
    RunTest(true, ut_SendRecvUSORF);
    RunTest(true, ut_SendRecvERROR);
    UnitTestMainEnd();
}
