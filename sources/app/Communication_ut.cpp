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
#include "Communication.h"
#include "DataTransferObject.h"

#define NUM_TEST_LOOPS 255

//--------------------------BUFFERS--------------------------
uint32_t g_currentTickCount;
bool g_taskJoined, g_taskStarted;
uint8_t g_crc;
std::thread::id g_masterThreadId, g_slaveThreadId;
static constexpr size_t MEMORYSIZE = 255;
uint8_t g_masterRxMemroy[MEMORYSIZE], g_slaveRxMemory[MEMORYSIZE];
bool g_comError = false;

//--------------------------MOCKING--------------------------
constexpr const std::array<const hal::Crc, hal::Crc::__ENUM__SIZE> hal::Factory<hal::Crc>::Container;
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

uint8_t hal::Crc::getCrc(uint8_t const* const data, const size_t length) const
{
    return g_crc;
}

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

void hal::UsartWithDma::enableReceiveTimeout(const size_t) const {}

size_t hal::UsartWithDma::receiveWithTimeout(uint8_t* const data, const size_t length,
                                             const uint32_t ticksToWait) const
{
    return receive(data, length, ticksToWait);
}

void hal::UsartWithDma::disableReceiveTimeout() const {}

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

    uint32_t a, b;

    auto rxDto = com::make_dto(a);
    auto txDto = com::make_dto(b);

    app::Communication<decltype(rxDto), decltype(txDto)> masterCom(hal::Factory<hal::UsartWithDma>::get<hal::Usart::
                                                                                                        MSCOM_IF>(),
                                                                   rxDto,
                                                                   txDto,
                                                                   [&](auto error)
                                                                   {
                                                                       if (error ==
                                                                           decltype(masterCom) ::ErrorCode::CRC_ERROR)
                                                                       {
                                                                           crcError = true;
                                                                       }
                                                                   });
    app::Communication<decltype(txDto), decltype(rxDto)> slaveCom(hal::Factory<hal::UsartWithDma>::get<hal::Usart::
                                                                                                       MSCOM_IF>(),
                                                                  txDto,
                                                                  rxDto);

    std::thread masterThread([&] {
                                 g_masterThreadId = std::this_thread::get_id();

                                 for (size_t i = 0; i < 100; i++) {
                                     masterCom.triggerTxTaskExecution();
                                     masterCom.triggerRxTaskExecution();
                                     std::this_thread::yield();
                                     std::this_thread::sleep_for(std::chrono::milliseconds(1));
                                 }
                             });
    std::thread slaveThread([&] {
                                g_slaveThreadId = std::this_thread::get_id();

                                for (size_t i = 0; i < 100; i++) {
                                    slaveCom.triggerTxTaskExecution();
                                    slaveCom.triggerRxTaskExecution();
                                    std::this_thread::yield();
                                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                                }
                            });

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    CHECK(crcError == false);

    g_crc = 0x11;

    std::this_thread::sleep_for(std::chrono::milliseconds(5));

    CHECK(crcError == true);

    masterThread.join();
    slaveThread.join();

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
                                                                                                        MSCOM_IF>(),
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
                                                                                                                    MSCOM_IF>(),
                                                                               masterRxDto,
                                                                               masterTxDto);
    app::Communication<decltype(masterTxDto), decltype(masterRxDto)> slaveCom(
                                                                              hal::Factory<hal::UsartWithDma>::get<hal
                                                                                                                   ::
                                                                                                                   Usart
                                                                                                                   ::
                                                                                                                   MSCOM_IF>(),
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
