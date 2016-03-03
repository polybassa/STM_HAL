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

BaseType_t xQueueGenericReceive(QueueHandle_t    xQueue,
                                void* const      pvBuffer,
                                TickType_t       xTicksToWait,
                                const BaseType_t xJustPeeking)
{
    return 1;
}

void RCC_GetClocksFreq(RCC_ClocksTypeDef*) {}

constexpr const hal::Usart& dev::DebugInterface::interface;

constexpr const std::array<const hal::Usart, hal::Usart::__ENUM__SIZE + 1> hal::Factory<hal::Usart>::Container;

//-------------------------TESTCASES-------------------------

// Global Variable for TEST
const dev::DebugInterface out;

int ut_Send_Hello(void)
{
    TestCaseBegin();

    std::string teststring = "HELLO_TEST";

    CHECK_NOT_MEMCMP(teststring.c_str(), output.data(), teststring.size());

    out.print(teststring.c_str());

    CHECK_MEMCMP(teststring.c_str(), output.data(), teststring.size());

    out.print("%s", teststring.c_str());

    CHECK_MEMCMP(teststring.c_str(), output.data(), teststring.size());

    TestCaseEnd();
}

int ut_Send_Number(void)
{
    TestCaseBegin();

    int testnum = 42;

    out.print("%d", testnum);

    std::string testout = std::to_string(testnum);

    CHECK_MEMCMP(testout.c_str(), output.data(), testout.size());

    TestCaseEnd();
}

int ut_Send_Two_Numbers(void)
{
    TestCaseBegin();

    int num1 = 42;
    float num2 = 42.0;

    out.print("%d%f", num1, num2);

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

    out.print("%d%f%s", num1, num2, teststring.c_str());

    std::string testout = (std::to_string(num1) + std::to_string(num2) + teststring);

    CHECK_MEMCMP(testout.c_str(), output.data(), testout.size());

    TestCaseEnd();
}

int ut_Send_to_much_Data(void)
{
    TestCaseBegin();

    std::string teststring =
        "HELLO_TESTHELLO_TESTHELLO_TESTHELLESTHELLO_TESTHELLO_TESTHELLESTHELLO_TESTHELLO_TESTHEL";

    teststring = teststring + teststring + teststring + teststring + teststring +
                 teststring + teststring;

    out.print("%s", teststring.c_str());

    std::string testout = teststring;

    CHECK_NOT_MEMCMP(testout.c_str(), output.data(), testout.size());

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
