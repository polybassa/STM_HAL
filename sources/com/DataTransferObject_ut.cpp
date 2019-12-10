// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 * Modified 2019 by Henning Mende
 */

#include <cmath>
#include <iostream>
#include <cstring>

#include "DataTransferObject.h"
#include "unittest.h"
#include "os_Task.h"

#define NUM_TEST_LOOPS 255

//--------------------------BUFFERS--------------------------
uint32_t g_currentTickCount;

#ifdef CRC_32BIT
uint32_t g_crc;
#else
uint8_t g_crc;
#endif // CRC_32BIT

//--------------------------MOCKING--------------------------
constexpr const std::array<const hal::Crc, hal::Crc::__ENUM__SIZE> hal::Factory<hal::Crc>::Container;

void os::ThisTask::enterCriticalSection(void){}

void os::ThisTask::exitCriticalSection(void) {}

uint32_t os::Task::getTickCount(void)
{
    return g_currentTickCount;
}

#ifdef CRC_32BIT
uint32_t hal::Crc::getCrc(uint8_t const* const data, const size_t length) const
{
    return g_crc;
}
#else
uint8_t hal::Crc::getCrc(uint8_t const* const data, const size_t length) const
{
    return g_crc;
}
#endif // CRC_32BIT

//-------------------------TESTCASES-------------------------

int ut_length(void)
{
    TestCaseBegin();

    g_crc = 0;
    g_currentTickCount = 0;

    uint32_t a = 1;
    uint16_t b = 2;
    uint8_t c = 3;

    com::DataTransferObject<uint8_t, uint32_t, uint16_t> dto(c, a, b);

    const size_t length = sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint8_t) + sizeof(uint32_t) + sizeof(g_crc);

    CHECK(dto.length() == length);

    TestCaseEnd();
}

int ut_tuple(void)
{
    TestCaseBegin();

    g_crc = 0;
    g_currentTickCount = 0;

    uint32_t a = 1;
    uint16_t b = 2;
    uint8_t c = 3;

    com::DataTransferObject<uint32_t, uint16_t, uint8_t> dto(a, b, c);

    auto& aRef = std::get<0>(dto.mTransferTuple);

    CHECK(aRef == a);
    CHECK(aRef == 1);

    aRef = 4;

    CHECK(a == 4);
    CHECK(a == aRef);

    TestCaseEnd();
}

int ut_tupleCopy(void)
{
    TestCaseBegin();

    g_crc = 0xAB;
    g_currentTickCount = 0xDEADBEEF;

    uint32_t a = 1;
    uint16_t b = 2;
    uint8_t c = 3;

    com::DataTransferObject<uint32_t, uint16_t, uint8_t> dto(a, b, c);

    dto.prepareForTx();

    uint8_t memory[dto.length()];
    std::memcpy(memory, dto.data(), dto.length());

    uint32_t dtoTick = *(reinterpret_cast<uint32_t*>(memory));

    CHECK(dtoTick == g_currentTickCount);

    uint32_t dtoA = *(reinterpret_cast<uint32_t*>(memory +
                                                  sizeof(uint32_t)));

    CHECK(dtoA == a);

    uint16_t dtoB = *(reinterpret_cast<uint16_t*>(memory +
                                                  sizeof(uint32_t) +
                                                  sizeof(uint32_t)));

    CHECK(dtoB == b);

    uint8_t dtoC = *(reinterpret_cast<uint8_t*>(memory +
                                                sizeof(uint32_t) +
                                                sizeof(uint32_t) +
                                                sizeof(uint16_t)));

    CHECK(dtoC == c);

    uint8_t dtoCRC = *(reinterpret_cast<uint8_t*>(memory +
                                                  sizeof(uint32_t) +
                                                  sizeof(uint32_t) +
                                                  sizeof(uint16_t) +
                                                  sizeof(uint8_t)));

    CHECK(dtoCRC == g_crc);

    TestCaseEnd();
}

int ut_tupleUpdate(void)
{
    TestCaseBegin();

    g_crc = 0x00;
    g_currentTickCount = 0x00;

    uint32_t a = 1;
    uint16_t b = 2;
    uint8_t c = 3;

    com::DataTransferObject<uint32_t, uint16_t, uint8_t> dto(a, b, c);

    uint8_t memory[dto.length()];

    uint32_t* dtoA = reinterpret_cast<uint32_t*>(memory +
                                                 sizeof(uint32_t));
    uint16_t* dtoB = reinterpret_cast<uint16_t*>(memory +
                                                 sizeof(uint32_t) +
                                                 sizeof(uint32_t));
    uint8_t* dtoC = reinterpret_cast<uint8_t*>(memory +
                                               sizeof(uint32_t) +
                                               sizeof(uint32_t) +
                                               sizeof(uint16_t));
    *dtoA = 0xDEADBEEF;
    *dtoB = 0xABCD;
    *dtoC = 0xEF;

    std::memcpy(dto.data(), memory, dto.length());

    dto.updateTuple();

    CHECK(a == *dtoA);
    CHECK(b == *dtoB);
    CHECK(c == *dtoC);

    TestCaseEnd();
}

int ut_tupleValid(void)
{
    TestCaseBegin();

    g_crc = 0x00;
    g_currentTickCount = 0x00;

    uint32_t a = 1;
    uint16_t b = 2;
    uint8_t c = 3;

    com::DataTransferObject<uint32_t, uint16_t, uint8_t> dto(a, b, c);
    dto.updateTuple();
    CHECK(dto.isValid() == true);
    TestCaseEnd();
}

int ut_makeDto(void)
{
    TestCaseBegin();

    g_crc = 0;
    g_currentTickCount = 0;

    uint32_t a = 1;
    uint16_t b = 2;
    uint8_t c = 3;

    auto dto = com::make_dto(a, b, c);

    auto& aRef = std::get<0>(dto.mTransferTuple);

    CHECK(aRef == a);
    CHECK(aRef == 1);

    aRef = 4;

    CHECK(a == 4);
    CHECK(a == aRef);

    TestCaseEnd();
}

int main(int argc, const char* argv[])
{
    UnitTestMainBegin();
    RunTest(true, ut_length);
    RunTest(true, ut_tuple);
    RunTest(true, ut_tupleCopy);
    RunTest(true, ut_tupleUpdate);
    RunTest(true, ut_tupleValid);
    RunTest(true, ut_makeDto);

    UnitTestMainEnd();
}
