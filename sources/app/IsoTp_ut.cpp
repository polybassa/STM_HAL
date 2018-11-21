// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */
#include <array>
#include <cstring>
#include <string>
#include "unittest.h"
#include "os_Task.h"
#include "IsoTp.h"
#include "Can.h"

static const int __attribute__((unused)) g_DebugZones = 0; //ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

#define NUM_TEST_LOOPS 255

//--------------------------BUFFERS--------------------------
std::array<CanTxMsg, 100> txBuffArray;
std::array<CanRxMsg, 100> rxBuffArray;
size_t txBuffCounter;
size_t rxBuffCounter;
bool timeOutTest = false;

//--------------------------MOCKING--------------------------
constexpr const std::array<const hal::Can, hal::Can::__ENUM__SIZE + 1> hal::Factory<hal::Can>::Container;

void os::ThisTask::sleep(const std::chrono::milliseconds ms)
{}

uint32_t os::Task::getTickCount(void)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
                                                                 std::chrono::high_resolution_clock::now().time_since_epoch())
           .count();
}

bool hal::Can::send(CanTxMsg& msg) const
{
    std::memcpy(&txBuffArray[txBuffCounter++], &msg, sizeof(CanTxMsg));
    return true;
}

bool hal::Can::receive(CanRxMsg& msg) const
{
    if (timeOutTest) {
        return false;
    }
    std::memcpy(&msg, &rxBuffArray[rxBuffCounter++], sizeof(CanRxMsg));
    return true;
}
//-------------------------TESTCASES-------------------------

int ut_SingleFrameTest(void)
{
    //============ PREPARE =====================
    TestCaseBegin();

    memset(txBuffArray.data(), 0, sizeof(txBuffArray));
    txBuffCounter = 0;

    memset(rxBuffArray.data(), 0, sizeof(rxBuffArray));
    rxBuffCounter = 0;

    //============ BEGIN TEST =====================

    constexpr const hal::Can& can = hal::Factory<hal::Can>::get<hal::Can::MAINCAN>();
    app::ISOTP testee(can, 0x7ff, 0x6ff);
    testee.send_Message(std::string_view("hello12", 7), std::chrono::milliseconds(200));

    CHECK(txBuffArray[0].StdId == 0x7ff);
    CHECK(txBuffArray[0].IDE == 0);
    CHECK(txBuffArray[0].DLC == 8);
    CHECK(txBuffArray[0].Data[0] == 0x07);
    CHECK(txBuffArray[0].Data[1] == 'h');
    CHECK(txBuffArray[0].Data[2] == 'e');
    CHECK(txBuffArray[0].Data[3] == 'l');
    CHECK(txBuffArray[0].Data[4] == 'l');
    CHECK(txBuffArray[0].Data[5] == 'o');
    CHECK(txBuffArray[0].Data[6] == '1');
    CHECK(txBuffArray[0].Data[7] == '2');
    TestCaseEnd();
}

int ut_FirstFrameTest(void)
{
    //============ PREPARE =====================
    TestCaseBegin();

    memset(txBuffArray.data(), 0, sizeof(txBuffArray));
    txBuffCounter = 0;

    memset(rxBuffArray.data(), 0, sizeof(rxBuffArray));
    rxBuffCounter = 0;

    rxBuffArray[0].StdId = 0x6ff;
    rxBuffArray[0].DLC = 0x3;
    rxBuffArray[0].Data[0] = 0x30;
    rxBuffArray[0].Data[1] = 0x00;
    rxBuffArray[0].Data[2] = 0x00;

    //============ BEGIN TEST =====================

    constexpr const hal::Can& can = hal::Factory<hal::Can>::get<hal::Can::MAINCAN>();
    app::ISOTP testee(can, 0x7ff, 0x6ff);
    auto result = testee.send_Message(std::string_view("deadbeef", 8), std::chrono::milliseconds(200));

    CHECK(result == 8);
    CHECK(txBuffArray[0].StdId == 0x7ff);
    CHECK(txBuffArray[0].IDE == 0);
    CHECK(txBuffArray[0].DLC == 8);
    CHECK(txBuffArray[0].Data[0] == 0x10);
    CHECK(txBuffArray[0].Data[1] == 0x08);
    CHECK(txBuffArray[0].Data[2] == 'd');
    CHECK(txBuffArray[0].Data[3] == 'e');
    CHECK(txBuffArray[0].Data[4] == 'a');
    CHECK(txBuffArray[0].Data[5] == 'd');
    CHECK(txBuffArray[0].Data[6] == 'b');
    CHECK(txBuffArray[0].Data[7] == 'e');

    CHECK(txBuffArray[1].StdId == 0x7ff);
    CHECK(txBuffArray[1].IDE == 0);
    CHECK(txBuffArray[1].DLC == 3);
    CHECK(txBuffArray[1].Data[0] == 0x21);
    CHECK(txBuffArray[1].Data[1] == 'e');
    CHECK(txBuffArray[1].Data[2] == 'f');

    TestCaseEnd();
}

int ut_FirstFrameIndexTest(void)
{
    //============ PREPARE =====================
    TestCaseBegin();

    memset(txBuffArray.data(), 0, sizeof(txBuffArray));
    txBuffCounter = 0;

    memset(rxBuffArray.data(), 0, sizeof(rxBuffArray));
    rxBuffCounter = 0;

    rxBuffArray[0].StdId = 0x6ff;
    rxBuffArray[0].DLC = 0x3;
    rxBuffArray[0].Data[0] = 0x30;
    rxBuffArray[0].Data[1] = 0x00;
    rxBuffArray[0].Data[2] = 0x00;

    //============ BEGIN TEST =====================

    constexpr const hal::Can& can = hal::Factory<hal::Can>::get<hal::Can::MAINCAN>();
    app::ISOTP testee(can, 0x7ff, 0x6ff);
    auto result = testee.send_Message(std::string_view("thatisanindex", 13), std::chrono::milliseconds(200));

    CHECK(result == 13);
    CHECK(txBuffArray[0].StdId == 0x7ff);
    CHECK(txBuffArray[0].IDE == 0);
    CHECK(txBuffArray[0].DLC == 8);
    CHECK(txBuffArray[0].Data[0] == 0x10);
    CHECK(txBuffArray[0].Data[1] == 0x0D);
    CHECK(txBuffArray[0].Data[2] == 't');
    CHECK(txBuffArray[0].Data[3] == 'h');
    CHECK(txBuffArray[0].Data[4] == 'a');
    CHECK(txBuffArray[0].Data[5] == 't');
    CHECK(txBuffArray[0].Data[6] == 'i');
    CHECK(txBuffArray[0].Data[7] == 's');

    CHECK(txBuffArray[1].StdId == 0x7ff);
    CHECK(txBuffArray[1].IDE == 0);
    CHECK(txBuffArray[1].DLC == 8);
    CHECK(txBuffArray[1].Data[0] == 0x21);
    CHECK(txBuffArray[1].Data[1] == 'a');
    CHECK(txBuffArray[1].Data[2] == 'n');
    CHECK(txBuffArray[1].Data[3] == 'i');
    CHECK(txBuffArray[1].Data[4] == 'n');
    CHECK(txBuffArray[1].Data[5] == 'd');
    CHECK(txBuffArray[1].Data[6] == 'e');
    CHECK(txBuffArray[1].Data[7] == 'x');

    TestCaseEnd();
}

int ut_ReceiveSingleFrame(void)
{
    //============ PREPARE =====================
    TestCaseBegin();

    memset(txBuffArray.data(), 0, sizeof(txBuffArray));
    txBuffCounter = 0;

    memset(rxBuffArray.data(), 0, sizeof(rxBuffArray));
    rxBuffCounter = 0;

    rxBuffArray[0].StdId = 0x6ff;
    rxBuffArray[0].DLC = 0x08;
    rxBuffArray[0].Data[0] = 0x07;
    rxBuffArray[0].Data[1] = 'h';
    rxBuffArray[0].Data[2] = 'e';
    rxBuffArray[0].Data[3] = 'l';
    rxBuffArray[0].Data[4] = 'l';
    rxBuffArray[0].Data[5] = 'o';
    rxBuffArray[0].Data[6] = '1';
    rxBuffArray[0].Data[7] = '2';

    //============ BEGIN TEST =====================

    constexpr const hal::Can& can = hal::Factory<hal::Can>::get<hal::Can::MAINCAN>();

    app::ISOTP testee(can, 0x7ff, 0x6ff);
    char buffer[7];
    size_t receivedNumberOfBytes = testee.receive_Message(buffer, sizeof(buffer), std::chrono::milliseconds(200));

    CHECK(receivedNumberOfBytes == 7);
    CHECK_MEMCMP(buffer, "hello12", 7);

    TestCaseEnd();
}

int ut_ReceiveFrameFalseId(void)
{
    //============ PREPARE =====================
    TestCaseBegin();

    memset(txBuffArray.data(), 0, sizeof(txBuffArray));
    txBuffCounter = 0;

    memset(rxBuffArray.data(), 0, sizeof(rxBuffArray));
    rxBuffCounter = 0;

    rxBuffArray[0].StdId = 0x7ff;
    rxBuffArray[0].DLC = 0x08;
    rxBuffArray[0].Data[0] = 0x07;
    rxBuffArray[0].Data[1] = 'h';
    rxBuffArray[0].Data[2] = 'e';
    rxBuffArray[0].Data[3] = 'l';
    rxBuffArray[0].Data[4] = 'l';
    rxBuffArray[0].Data[5] = 'o';
    rxBuffArray[0].Data[6] = '1';
    rxBuffArray[0].Data[7] = '2';

    //============ BEGIN TEST =====================

    constexpr const hal::Can& can = hal::Factory<hal::Can>::get<hal::Can::MAINCAN>();

    app::ISOTP testee(can, 0x7ff, 0x6ff);
    char buffer[7];
    size_t receivedNumberOfBytes = testee.receive_Message(buffer, sizeof(buffer), std::chrono::milliseconds(200));

    CHECK(receivedNumberOfBytes == 0);
    CHECK_MEMCMP(buffer, "", 1);

    TestCaseEnd();
}

int ut_ReceiveFirstFrame(void)
{
    //============ PREPARE =====================
    TestCaseBegin();

    memset(txBuffArray.data(), 0, sizeof(txBuffArray));
    txBuffCounter = 0;

    memset(rxBuffArray.data(), 0, sizeof(rxBuffArray));
    rxBuffCounter = 0;

    // is needed to simulate received first frame
    rxBuffArray[0].StdId = 0x6ff;
    rxBuffArray[0].IDE = 0x00;
    rxBuffArray[0].RTR = 0x00;
    rxBuffArray[0].DLC = 0x08;
    rxBuffArray[0].Data[0] = 0x10;
    rxBuffArray[0].Data[1] = 0x08;
    rxBuffArray[0].Data[2] = 0x64;
    rxBuffArray[0].Data[3] = 0x65;
    rxBuffArray[0].Data[4] = 0x61;
    rxBuffArray[0].Data[5] = 0x64;
    rxBuffArray[0].Data[6] = 0x62;
    rxBuffArray[0].Data[7] = 0x65;

    // is needed to simulate received consecutive frames
    rxBuffArray[1].StdId = 0x6ff;
    rxBuffArray[1].IDE = 0x00;
    rxBuffArray[1].RTR = 0x00;
    rxBuffArray[1].DLC = 0x03;
    rxBuffArray[1].Data[0] = 0x21;
    rxBuffArray[1].Data[1] = 0x65;
    rxBuffArray[1].Data[2] = 0x66;

    //============ BEGIN TEST =====================

    constexpr const hal::Can& can = hal::Factory<hal::Can>::get<hal::Can::MAINCAN>();

    app::ISOTP testee(can, 0x7ff, 0x6ff);

    char buffer[10];

    size_t receivedNumberOfBytes = testee.receive_Message(buffer, sizeof(buffer), std::chrono::milliseconds(200));

    // check flow control
    CHECK(txBuffArray[0].StdId == 0x7ff);
    CHECK(txBuffArray[0].DLC == 3);
    CHECK(txBuffArray[0].Data[0] == 0x30);
    CHECK(txBuffArray[0].Data[1] == 0x01);
    CHECK(txBuffArray[0].Data[2] == 0x01);

    CHECK_MEMCMP(buffer, "deadbeef", 8);
    CHECK(receivedNumberOfBytes == 8);

    TestCaseEnd();
}

int ut_RunTestWithTwoISOTPObjects(void)
{
    //============ PREPARE =====================
    TestCaseBegin();

    memset(txBuffArray.data(), 0, sizeof(txBuffArray));
    txBuffCounter = 0;

    memset(rxBuffArray.data(), 0, sizeof(rxBuffArray));
    rxBuffCounter = 0;

    //============ BEGIN TEST =====================

    constexpr const hal::Can& can = hal::Factory<hal::Can>::get<hal::Can::MAINCAN>();

    app::ISOTP isotpSender(can, 0x7ff, 0x6ff);
    app::ISOTP isotpEmpfaenger(can, 0x6ff, 0x7ff);
    size_t NumberOfBytesSended =
        isotpSender.send_Message(std::string_view("hello12", 7), std::chrono::milliseconds(200));

    CHECK(NumberOfBytesSended == 7);
    CHECK(txBuffArray[0].StdId == 0x7ff);
    CHECK(txBuffArray[0].IDE == 0);
    CHECK(txBuffArray[0].DLC == 8);
    CHECK(txBuffArray[0].Data[0] == 0x07);
    CHECK(txBuffArray[0].Data[1] == 'h');
    CHECK(txBuffArray[0].Data[2] == 'e');
    CHECK(txBuffArray[0].Data[3] == 'l');
    CHECK(txBuffArray[0].Data[4] == 'l');
    CHECK(txBuffArray[0].Data[5] == 'o');
    CHECK(txBuffArray[0].Data[6] == '1');
    CHECK(txBuffArray[0].Data[7] == '2');

    memcpy(&rxBuffArray[0], &txBuffArray[0], std::min(sizeof(CanRxMsg), sizeof(CanTxMsg)));
    char buffer[7];
    size_t NumberOfBytesReceived = isotpEmpfaenger.receive_Message(buffer,
                                                                   sizeof(buffer),
                                                                   std::chrono::milliseconds(200));

    CHECK(NumberOfBytesReceived == 7);
    CHECK_MEMCMP(buffer, "hello12", 7);

    TestCaseEnd();
}

int ut_ReceiveToLongSingleFrameTest(void)
{
    //============ PREPARE =====================
    TestCaseBegin();

    memset(txBuffArray.data(), 0, sizeof(txBuffArray));
    txBuffCounter = 0;

    memset(rxBuffArray.data(), 0, sizeof(rxBuffArray));
    rxBuffCounter = 0;

    rxBuffArray[0].StdId = 0x6ff;
    rxBuffArray[0].DLC = 0x08;
    rxBuffArray[0].Data[0] = 0x08;
    rxBuffArray[0].Data[1] = 'h';
    rxBuffArray[0].Data[2] = 'e';
    rxBuffArray[0].Data[3] = 'l';
    rxBuffArray[0].Data[4] = 'l';
    rxBuffArray[0].Data[5] = 'o';
    rxBuffArray[0].Data[6] = '1';
    rxBuffArray[0].Data[7] = '2';
    rxBuffArray[0].Data[8] = '5';

    //============ BEGIN TEST =====================

    constexpr const hal::Can& can = hal::Factory<hal::Can>::get<hal::Can::MAINCAN>();

    app::ISOTP testee(can, 0x7ff, 0x6ff);
    char buffer[7];
    size_t receivedNumberOfBytes = testee.receive_Message(buffer, sizeof(buffer), std::chrono::milliseconds(200));

    CHECK(receivedNumberOfBytes == 0);
    CHECK_MEMCMP(buffer, "", 1);

    TestCaseEnd();
}

int ut_length_Bigger_than_Buffer(void)
{
    //============ PREPARE =====================
    TestCaseBegin();

    memset(txBuffArray.data(), 0, sizeof(txBuffArray));
    txBuffCounter = 0;

    memset(rxBuffArray.data(), 0, sizeof(rxBuffArray));
    rxBuffCounter = 0;

    // is needed to simulate received first frame
    rxBuffArray[0].StdId = 0x6ff;
    rxBuffArray[0].IDE = 0x00;
    rxBuffArray[0].RTR = 0x00;
    rxBuffArray[0].DLC = 0x08;
    rxBuffArray[0].Data[0] = 0x10;
    rxBuffArray[0].Data[1] = 0x08;
    rxBuffArray[0].Data[2] = 0x64;
    rxBuffArray[0].Data[3] = 0x65;
    rxBuffArray[0].Data[4] = 0x61;
    rxBuffArray[0].Data[5] = 0x64;
    rxBuffArray[0].Data[6] = 0x62;
    rxBuffArray[0].Data[7] = 0x65;

    // is needed to simulate received consecutive frames
    rxBuffArray[1].StdId = 0x6ff;
    rxBuffArray[1].IDE = 0x00;
    rxBuffArray[1].RTR = 0x00;
    rxBuffArray[1].DLC = 0x03;
    rxBuffArray[1].Data[0] = 0x21;
    rxBuffArray[1].Data[1] = 0x65;
    rxBuffArray[1].Data[2] = 0x66;

    //============ BEGIN TEST =====================

    constexpr const hal::Can& can = hal::Factory<hal::Can>::get<hal::Can::MAINCAN>();

    app::ISOTP isotpReceiver(can, 0x7ff, 0x6ff);
    char buffer[7];
    size_t NumberOfBytesReceived = isotpReceiver.receive_Message(buffer, 4, std::chrono::milliseconds(200));

    // check flow control overflow
    CHECK(txBuffArray[0].Data[0] == 0x32);
    CHECK(txBuffArray[0].Data[1] == 0x01);
    CHECK(txBuffArray[0].Data[2] == 0x01);

    CHECK_MEMCMP(buffer, "", 1);
    CHECK(NumberOfBytesReceived == 0);

    TestCaseEnd();
}

int ut_message_Bigger_than_Buffer(void)
{
    //============ PREPARE =====================
    TestCaseBegin();

    memset(txBuffArray.data(), 0, sizeof(txBuffArray));
    txBuffCounter = 0;

    memset(rxBuffArray.data(), 0, sizeof(rxBuffArray));
    rxBuffCounter = 0;

    // is needed to simulate received first frame
    rxBuffArray[0].StdId = 0x6ff;
    rxBuffArray[0].IDE = 0x00;
    rxBuffArray[0].RTR = 0x00;
    rxBuffArray[0].DLC = 0x08;
    rxBuffArray[0].Data[0] = 0x10;
    rxBuffArray[0].Data[1] = 0x09;
    rxBuffArray[0].Data[2] = 0x64;
    rxBuffArray[0].Data[3] = 0x65;
    rxBuffArray[0].Data[4] = 0x61;
    rxBuffArray[0].Data[5] = 0x64;
    rxBuffArray[0].Data[6] = 0x62;
    rxBuffArray[0].Data[7] = 0x65;

    // is needed to simulate received consecutive frames
    rxBuffArray[1].StdId = 0x6ff;
    rxBuffArray[1].IDE = 0x00;
    rxBuffArray[1].RTR = 0x00;
    rxBuffArray[1].DLC = 0x03;
    rxBuffArray[1].Data[0] = 0x21;
    rxBuffArray[1].Data[1] = 0x65;
    rxBuffArray[1].Data[2] = 0x66;
    rxBuffArray[1].Data[3] = 0x66;

    //============ BEGIN TEST =====================

    constexpr const hal::Can& can = hal::Factory<hal::Can>::get<hal::Can::MAINCAN>();

    app::ISOTP isotpReceiver(can, 0x7ff, 0x6ff);
    char buffer[8];
    size_t NumberOfBytesReceived = isotpReceiver.receive_Message(buffer,
                                                                 sizeof(buffer),
                                                                 std::chrono::milliseconds(200));

    // check flow control overflow
    CHECK(txBuffArray[0].Data[0] == 0x32);
    CHECK(txBuffArray[0].Data[1] == 0x01);
    CHECK(txBuffArray[0].Data[2] == 0x01);

    CHECK_MEMCMP(buffer, "", 1);
    CHECK(NumberOfBytesReceived == 0);

    TestCaseEnd();
}

int ut_Timeout(void)
{
    //============ PREPARE =====================
    TestCaseBegin();
    uint32_t startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                                                                               std::chrono::high_resolution_clock::now().time_since_epoch())
                         .count();
    timeOutTest = true;
    memset(txBuffArray.data(), 0, sizeof(txBuffArray));
    txBuffCounter = 0;

    memset(rxBuffArray.data(), 0, sizeof(rxBuffArray));
    rxBuffCounter = 0;

    rxBuffArray[0].StdId = 0x6ff;
    rxBuffArray[0].DLC = 0x08;
    rxBuffArray[0].Data[0] = 0x07;
    rxBuffArray[0].Data[1] = 'h';
    rxBuffArray[0].Data[2] = 'e';
    rxBuffArray[0].Data[3] = 'l';
    rxBuffArray[0].Data[4] = 'l';
    rxBuffArray[0].Data[5] = 'o';
    rxBuffArray[0].Data[6] = '1';
    rxBuffArray[0].Data[7] = '2';

    //============ BEGIN TEST =====================

    constexpr const hal::Can& can = hal::Factory<hal::Can>::get<hal::Can::MAINCAN>();

    app::ISOTP testee(can, 0x7ff, 0x6ff);
    char buffer[7];
    size_t receivedNumberOfBytes = testee.receive_Message(buffer, sizeof(buffer), std::chrono::milliseconds(10));
    uint32_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                                                                                 std::chrono::high_resolution_clock::now().time_since_epoch())
                           .count();

    CHECK((currentTime - startTime) >= 10);
    CHECK(receivedNumberOfBytes == 0);
    CHECK_MEMCMP(buffer, "", 1);

    timeOutTest = false;
    TestCaseEnd();
}

int main(int argc, const char* argv[])
{
    UnitTestMainBegin();
    RunTest(true, ut_SingleFrameTest);
    RunTest(true, ut_FirstFrameTest);
    RunTest(true, ut_ReceiveSingleFrame);
    RunTest(true, ut_ReceiveFirstFrame);
    RunTest(true, ut_ReceiveFrameFalseId);
    RunTest(true, ut_FirstFrameIndexTest);
    RunTest(true, ut_RunTestWithTwoISOTPObjects);
    RunTest(true, ut_ReceiveToLongSingleFrameTest);
    RunTest(true, ut_length_Bigger_than_Buffer);
    RunTest(true, ut_message_Bigger_than_Buffer);
    RunTest(true, ut_Timeout);
    UnitTestMainEnd();
}
