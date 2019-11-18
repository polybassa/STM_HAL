// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 * Modified 2019 by Henning Mende
 */

#include <cmath>
#include <thread>
#include <iostream>
#include <cstring>
#include "unittest.h"
#include "Communication.h"
#include "DataTransferObject.h"

#define NUM_TEST_LOOPS 255

//--------------------------BUFFERS--------------------------
uint32_t g_currentTickCount;
bool g_taskJoined, g_taskStarted;
std::thread::id g_masterThreadId, g_slaveThreadId;
static constexpr size_t MEMORYSIZE = 255;
uint8_t g_masterRxMemroy[MEMORYSIZE], g_slaveRxMemory[MEMORYSIZE];
bool g_comError = false;

#ifdef CRC_32BIT
uint32_t g_crc;
#else
uint8_t g_crc;
#endif // CRC_32BIT
//--------------------------MOCKING--------------------------
constexpr const std::array<const hal::Crc, hal::Crc::__ENUM__SIZE> hal::Factory<hal::Crc>::Container;
constexpr const std::array<const hal::UsartWithDma,
                           hal::UsartWithDma::NUMBER_OF_INSTANCES> hal::Factory<hal::UsartWithDma>::Container;
constexpr const std::array<const hal::Dma, hal::Dma::__ENUM__SIZE + 1> hal::Factory<hal::Dma>::Container;

#ifdef NVIC_ABSTRACTION
constexpr const std::array<const hal::Nvic, hal::Nvic::__ENUM__SIZE + 1> hal::Factory<hal::Nvic>::Container;
constexpr const std::array<const hal::Usart, hal::Usart::__ENUM__SIZE> hal::Factory<hal::Usart>::Container;
#else
constexpr const std::array<const hal::Usart, hal::Usart::__ENUM__SIZE + 1> hal::Factory<hal::Usart>::Container;
#endif

#ifndef COM_INTERFACE
#error COM_INTERFACE must be defined with a member of hal::Usart::Description!
#endif

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

size_t hal::UsartWithDma::send(uint8_t const* const data, const size_t length, const uint32_t ticksToWait) const
{
    if (std::this_thread::get_id() == g_masterThreadId) {
        std::memcpy(g_slaveRxMemory, data, length);
    } else {
        std::memcpy(g_masterRxMemroy, data, length);
    }
    return g_comError ? 0 : length;
}

size_t hal::UsartWithDma::receive(uint8_t* const data, const size_t length, const uint32_t ticksToWait) const
{
    if (std::this_thread::get_id() == g_slaveThreadId) {
        std::memcpy(data, g_slaveRxMemory, length);
    } else {
        std::memcpy(data, g_masterRxMemroy, length);
    }
    return g_comError ? 0 : length;
}

#ifndef NO_USART_HARDWARE_TIMEOUT
void hal::UsartWithDma::enableReceiveTimeout(const size_t) const {}
#endif

size_t hal::UsartWithDma::receiveWithTimeout(uint8_t* const data, const size_t length,
                                             const uint32_t ticksToWait) const
{
    return receive(data, length, ticksToWait);
}

#ifndef NO_USART_HARDWARE_TIMEOUT
void hal::UsartWithDma::disableReceiveTimeout() const {}
#endif

void os::ThisTask::enterCriticalSection() {}

void os::ThisTask::exitCriticalSection() {}

//-------------------------TESTCASES-------------------------

int ut_CrcError(void)
{
    TestCaseBegin();

    g_crc = 0x00;
    g_currentTickCount = 0;
    g_comError = false;

    auto crcError = false;

    uint32_t a = 0, b = 0;

    auto rxDto = com::make_dto(a);
    auto txDto = com::make_dto(b);

    app::Communication<decltype(rxDto), decltype(txDto)> masterCom(hal::Factory<hal::UsartWithDma>::get<hal::Usart::
                                                                                                        COM_INTERFACE>(),
                                                                   rxDto,
                                                                   txDto,
                                                                   [&](auto error)
        {
                                                                   if (error == com::ErrorCode::CRC_ERROR) {
                                                                       crcError = true;
                                                                   }
        });
    app::Communication<decltype(txDto), decltype(rxDto)> slaveCom(hal::Factory<hal::UsartWithDma>::get<hal::Usart::
                                                                                                       COM_INTERFACE>(),
                                                                  txDto,
                                                                  rxDto);

    masterCom.triggerTxTaskExecution();
    masterCom.triggerRxTaskExecution();
    slaveCom.triggerTxTaskExecution();
    slaveCom.triggerRxTaskExecution();

    masterCom.triggerTxTaskExecution();
    slaveCom.triggerTxTaskExecution();
    masterCom.triggerRxTaskExecution();
    slaveCom.triggerRxTaskExecution();

    CHECK(crcError == false);

    g_crc = 0x11;

    masterCom.triggerRxTaskExecution();

    CHECK(crcError == true);

    TestCaseEnd();
}

int ut_DeepSleep(void)
{
    TestCaseBegin();

    g_crc = 0;
    g_currentTickCount = 0;
    g_comError = false;
    memset(g_masterRxMemroy, 0, MEMORYSIZE);
    memset(g_slaveRxMemory, 0, MEMORYSIZE);

    CHECK(false == g_taskJoined);
    CHECK(false == g_taskStarted);

    uint32_t a, b;

    auto rxDto = com::make_dto(a);
    auto txDto = com::make_dto(b);

    app::Communication<decltype(rxDto), decltype(txDto)> masterCom(
                                                                   hal::Factory<hal::UsartWithDma>::get<hal::Usart::
                                                                                                        COM_INTERFACE>(),
                                                                   rxDto,
                                                                   txDto);

    os::DeepSleepController::enterGlobalDeepSleep();

    CHECK(true == g_taskJoined);

    os::DeepSleepController::exitGlobalDeepSleep();

    CHECK(true == g_taskStarted);
    TestCaseEnd();
}

int ut_ValueExchange(void)
{
    TestCaseBegin();

    g_crc = 0;
    g_currentTickCount = 0;
    g_comError = false;

    uint32_t mRx, mTx, sRx, sTx;

    auto masterRxDto = com::make_dto(mRx);
    auto masterTxDto = com::make_dto(mTx);
    auto slaveRxDto = com::make_dto(sRx);
    auto slaveTxDto = com::make_dto(sTx);

    app::Communication<decltype(masterRxDto), decltype(masterTxDto)> masterCom(
                                                                               hal::Factory<hal::UsartWithDma>::get<hal
                                                                                                                    ::
                                                                                                                    Usart
                                                                                                                    ::
                                                                                                                    COM_INTERFACE>(),
                                                                               masterRxDto,
                                                                               masterTxDto);
    app::Communication<decltype(masterTxDto), decltype(masterRxDto)> slaveCom(
                                                                              hal::Factory<hal::UsartWithDma>::get<hal
                                                                                                                   ::
                                                                                                                   Usart
                                                                                                                   ::
                                                                                                                   COM_INTERFACE>(),
                                                                              slaveRxDto,
                                                                              slaveTxDto);

    std::thread masterThread([&] {
                             g_masterThreadId = std::this_thread::get_id();

                             for (size_t i = 0; i < 1000; i++) {
                                 masterCom.triggerTxTaskExecution();
                                 masterCom.triggerRxTaskExecution();
                                 std::this_thread::yield();
                                 std::this_thread::sleep_for(std::chrono::milliseconds(1));
                             }
        });
    std::thread slaveThread([&] {
                            g_slaveThreadId = std::this_thread::get_id();

                            for (size_t i = 0; i < 1000; i++) {
                                slaveCom.triggerTxTaskExecution();
                                slaveCom.triggerRxTaskExecution();
                                std::this_thread::yield();
                                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                            }
        });

    for (size_t i = 0; i < 20; i++) {
        mTx = i;
        sTx = i;
        std::this_thread::sleep_for(std::chrono::milliseconds(3));
        CHECK(mRx == i);
        CHECK(sRx == i);
        g_currentTickCount += 5;
    }

    masterThread.join();
    slaveThread.join();

    TestCaseEnd();
}

int main(int argc, const char* argv[])
{
    UnitTestMainBegin();
    RunTest(true, ut_DeepSleep);
    RunTest(true, ut_CrcError);
    RunTest(true, ut_ValueExchange);
    UnitTestMainEnd();
}
