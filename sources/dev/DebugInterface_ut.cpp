// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include <string>
#include "DebugInterface.h"
#include "Usart.h"
#include "unittest.h"
#include <array>
#include <cstring>

#define NUM_TEST_LOOPS 255

//--------------------------BUFFERS--------------------------
std::array<uint8_t, 1024> output;

//--------------------------MOCKING--------------------------
size_t hal::Usart::send(const uint8_t* str, size_t n) const
{
    std::memcpy(output.data(), str, n);
    return n;
}

size_t hal::Usart::receiveAvailableData(uint8_t* const data, const size_t length) const
{
    return 0;
}
bool hal::Usart::isReadyToReceive(void) const
{
    return false;
}

bool hal::Usart::isInitalized(void) const
{
    return true;
}

void vQueueDelete(QueueHandle_t xQueue) {}

QueueHandle_t xQueueCreateMutex(const uint8_t ucQueueType)
{
    return nullptr;
}

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

void RCC_GetClocksFreq(RCC_ClocksTypeDef*) {}

constexpr const hal::Usart& dev::DebugInterface::interface;
#ifdef __STM32F4xx_H // usage of stm32 f4 hal
constexpr const std::array<const hal::Usart, hal::Usart::__ENUM__SIZE> hal::Factory<hal::Usart>::Container;
constexpr const std::array<const hal::Nvic, hal::Nvic::__ENUM__SIZE + 1> hal::Factory<hal::Nvic>::Container;
#else
constexpr const std::array<const hal::Usart, hal::Usart::__ENUM__SIZE + 1> hal::Factory<hal::Usart>::Container;
#endif

//-------------------------TESTCASES-------------------------

// Global Variable for TEST
const dev::DebugInterface out;

int ut_Send_Hello(void)
{
    TestCaseBegin();

    std::string teststring = "HELLO_TEST";

    CHECK_NOT_MEMCMP(teststring.c_str(), output.data(), teststring.size());

    out.printf(teststring.c_str());

    CHECK_MEMCMP(teststring.c_str(), output.data(), teststring.size());

    out.printf("%s", teststring.c_str());

    CHECK_MEMCMP(teststring.c_str(), output.data(), teststring.size());

    TestCaseEnd();
}

int ut_Send_Number(void)
{
    TestCaseBegin();

    int testnum = 42;

    out.printf("%d", testnum);

    std::string testout = std::to_string(testnum);

    CHECK_MEMCMP(testout.c_str(), output.data(), testout.size());

    TestCaseEnd();
}

int ut_Send_Two_Numbers(void)
{
    TestCaseBegin();

    int num1 = 42;
    float num2 = 42.0;

    out.printf("%d%f", num1, num2);

    std::string testout = (std::to_string(num1) + std::to_string(num2));

    CHECK_MEMCMP(testout.c_str(), output.data(), testout.size());

    TestCaseEnd();
}

int ut_Send_Multiple_Data(void)
{
    TestCaseBegin();

    int num1 = 42;
    float num2 = 42.0;
    std::string teststring = "HELLO_TEST";

    out.printf("%d%f%s", num1, num2, teststring.c_str());

    std::string testout = (std::to_string(num1) + std::to_string(num2) + teststring);

    CHECK_MEMCMP(testout.c_str(), output.data(), testout.size());

    TestCaseEnd();
}

int ut_Send_to_much_Data(void)
{
    TestCaseBegin();

    std::string teststring =
        "HELLO_TESTHELLHELLO_TESTHELLO_TESTHELLO_TESTHELLO_TESTHELLO_TESTHELLESTHELLO_TESTHELLO_TESTHELLESTHELLO_TESTHELLO_TESTHELO_TESTHELLO_TESTHELLO_TESTHELLO_TESTHELLESTHELLO_TESTHELLO_TESTHELLESTHELLO_TESTHELLO_TESTHEL";

    teststring = teststring + teststring + teststring + teststring + teststring +
                 teststring + teststring + teststring;

    out.printf("%s", teststring.c_str());

    std::string testout = teststring;

    CHECK_NOT_MEMCMP(teststring.c_str(), output.data(), std::min(output.size(), teststring.size()));

    TestCaseEnd();
}

int main(int argc, const char* argv[])
{
    UnitTestMainBegin();
    RunTest(true, ut_Send_Hello);
    RunTest(true, ut_Send_Number);
    RunTest(true, ut_Send_Two_Numbers);
    RunTest(true, ut_Send_Multiple_Data);
    RunTest(true, ut_Send_to_much_Data);

    UnitTestMainEnd();
}
