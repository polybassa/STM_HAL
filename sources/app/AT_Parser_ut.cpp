// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "unittest.h"
#include "AT_Parser.h"
#include <condition_variable>
#include <thread>
#include <mutex>
#include "os_Queue.h"

static const int __attribute__((unused)) g_DebugZones = 0; //ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using os::Mutex;

#define NUM_TEST_LOOPS 255

//--------------------------BUFFERS--------------------------
bool g_semaphoreGiven = false;

//--------------------------MOCKING--------------------------

void os::ThisTask::sleep(const std::chrono::milliseconds ms)
{
    std::this_thread::sleep_for(ms);
}

Mutex::Mutex(void) :
    mMutexHandle((SemaphoreHandle_t) new int)
{
    this->give();
}

Mutex::Mutex(Mutex&& rhs) :
    mMutexHandle(rhs.mMutexHandle)
{
    rhs.mMutexHandle = nullptr;
}

Mutex& Mutex::operator=(Mutex&& rhs)
{
    mMutexHandle = rhs.mMutexHandle;
    rhs.mMutexHandle = nullptr;
    return *this;
}

Mutex::~Mutex(void)
{
    delete (int*)mMutexHandle;
}

bool Mutex::take(uint32_t ticksToWait) const
{
    if (*this && (*(reinterpret_cast<int*>(mMutexHandle)) == 1)) {
        *(reinterpret_cast<int*>(mMutexHandle)) = 0;
        return true;
    }

    return false;
}

bool Mutex::give(void) const
{
    if (*this) {
        *(reinterpret_cast<int*>(mMutexHandle)) = 1;
        return true;
    }
    return false;
}

Mutex::operator bool() const
{
    return mMutexHandle != nullptr;
}

QueueHandle_t xQueueGenericCreate(const UBaseType_t uxQueueLength,
                                  const UBaseType_t uxItemSize,
                                  const uint8_t     ucQueueType)
{
    return (QueueHandle_t)malloc(10);
}

void vQueueDelete(QueueHandle_t xQueue)
{
    delete xQueue;
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
    if (g_semaphoreGiven) {
        g_semaphoreGiven = false;
        return true;
    }
    return false;
}

BaseType_t xQueueGenericReset(QueueHandle_t xQueue, BaseType_t xNewQueue)
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
    g_semaphoreGiven = true;
    return g_semaphoreGiven;
}

BaseType_t xQueueReceive(QueueHandle_t xQueue, void* const pvBuffer, TickType_t xTicksToWait)
{
    return 1;
}

//-------------------------TESTCASES-------------------------

int ut_BasicTest(void)
{
    TestCaseBegin();

    std::condition_variable cv;
    std::mutex cv_m;
    int i = 0;

    static std::string testString = " ";

    std::function<size_t(uint8_t*, const size_t, std::chrono::milliseconds)> recv =
        [&](uint8_t* data, const size_t length, std::chrono::milliseconds timeout) -> size_t {
            static size_t position = 0;
            size_t i = 0;

            if (testString.length() - position <= 0) {
                Trace(ZONE_INFO, "recv Sleep;\r\n");
                std::unique_lock<std::mutex> lk(cv_m);
                cv.wait_for(lk, timeout, [&] {
                return testString.length() - position > 0;
            });
            }

            for ( ; i < length && position < testString.length(); i++) {
                *data = testString[position++];
            }
            return i;
        };

    std::function<size_t(std::string_view, std::chrono::milliseconds)> send =
        [](std::string_view in, std::chrono::milliseconds) -> size_t {
            return in.length();
        };

    app::ATParser parser(recv);

    auto testee1 = std::shared_ptr<app::AT>(new app::ATCmd("CMD_1", "REQ1", "RESP1"));
    auto testee2 = std::shared_ptr<app::AT>(new app::ATCmd("CMD_2", "REQ2", "RESP2"));
    auto testee3 = std::shared_ptr<app::AT>(new app::ATCmd("CMD_3", "REQ3", "REsp3"));
    auto testee4 = std::shared_ptr<app::AT>(new app::ATCmdOK());
    auto testee5 = std::shared_ptr<app::AT>(new app::ATCmdERROR());

    parser.registerAtCommand(testee1.get());
    parser.registerAtCommand(testee2.get());
    parser.registerAtCommand(testee3.get());
    parser.registerAtCommand(testee4.get());
    parser.registerAtCommand(testee5.get());

    auto send1 = [&](int j = 2)
                 {
                     Trace(ZONE_INFO, "Sleep %d\r\n", j);
                     {
                         std::unique_lock<std::mutex> lk(cv_m);
                         cv.wait(lk, [&] {
                return j == i;
            });

                         Trace(ZONE_INFO, "Hello %d\r\n", j);
                         testString += "\rOK\r";
                     }
                     auto ret =
                         std::dynamic_pointer_cast<app::ATCmd>(testee3)->send(send, std::chrono::milliseconds(2000));
                     CHECK(ret == app::AT::Return_t::FINISHED);
                 };

    auto send2 = [&](int j = 0)
                 {
                     Trace(ZONE_INFO, "Sleep %d\r\n", j);
                     {
                         std::unique_lock<std::mutex> lk(cv_m);
                         cv.wait(lk, [&] {
                return j == i;
            });
                         testString += "\rRESP2\rOK\r";
                         Trace(ZONE_INFO, "Hello %d\r\n", j);
                     }
                     auto ret =
                         std::dynamic_pointer_cast<app::ATCmd>(testee2)->send(send, std::chrono::milliseconds(2000));
                     CHECK(ret == app::AT::Return_t::FINISHED);
                 };

    auto parse = [&](int j = 1)
                 {
                     Trace(ZONE_INFO, "Sleep %d\r\n", j);
                     {
                         std::unique_lock<std::mutex> lk(cv_m);
                         cv.wait(lk, [&] {
                return j == i;
            });
                     }
                     Trace(ZONE_INFO, "Hello %d\r\n", j);

                     for (auto i = 0; i < 2; i++) {
                         parser.parse(std::chrono::milliseconds(100));
                     }
                     Trace(ZONE_INFO, "parser END \r\n");
                 };

    auto signals = [&]
                   {
                       Trace(ZONE_INFO, "Signal");

                       std::this_thread::sleep_for(std::chrono::milliseconds(10));
                       {
                           std::lock_guard<std::mutex> lk(cv_m);
                           i = 0;
                       }
                       cv.notify_all();

                       std::this_thread::sleep_for(std::chrono::milliseconds(10));

                       {
                           std::lock_guard<std::mutex> lk(cv_m);
                           i = 1;
                       }
                       cv.notify_all();
                       std::this_thread::sleep_for(std::chrono::milliseconds(100));

                       {
                           std::lock_guard<std::mutex> lk(cv_m);
                           i = 2;
                       }
                       cv.notify_all();
                       std::this_thread::sleep_for(std::chrono::milliseconds(10));

                       {
                           std::lock_guard<std::mutex> lk(cv_m);
                           i = 3;
                       }
                       cv.notify_all();
                       std::this_thread::sleep_for(std::chrono::milliseconds(10));
                   };

    std::thread t0(send2, 1), t1(parse, 0), t2(send1, 2), t3(signals);
    Trace(ZONE_INFO, "Wait \r\n");

    t0.join();
    t1.join();
    t2.join();
    t3.join();

    TestCaseEnd();
}

int ut_ATParserURCTest(void)
{
    TestCaseBegin();

    static std::string testString = "\rRESP2\rOK\rERROR\rUURC1: 1,5\rOK\r\nPESUURC2: 4711,128\r\nOK";
    static auto pos = testString.begin();

    std::function<size_t(uint8_t*, const size_t, std::chrono::milliseconds)> recv =
        [&](uint8_t* data, const size_t length, std::chrono::milliseconds) -> size_t {
            size_t i = 0;
            for ( ; i < length && pos != testString.end(); i++) {
                *data = *pos++;
            }
            return i;
        };

    std::function<size_t(std::string_view, std::chrono::milliseconds)> send =
        [](std::string_view in, std::chrono::milliseconds) -> size_t {
            return in.length();
        };

    size_t urc1sock = 0;
    size_t urc1bytes = 0;

    std::function<void(size_t, size_t)> urc1Callback = [&](size_t sock, size_t databytes){
                                                           urc1sock = sock;
                                                           urc1bytes = databytes;
                                                       };

    size_t urc2sock = 0;
    size_t urc2bytes = 0;

    std::function<void(size_t, size_t)> urc2Callback = [&](size_t sock, size_t databytes){
                                                           urc2sock = sock;
                                                           urc2bytes = databytes;
                                                       };

    app::ATParser parser(recv);

    auto testee1 = std::shared_ptr<app::AT>(new app::ATCmdURC("CMD_1", "UURC1:", urc1Callback));
    auto testee2 = std::shared_ptr<app::AT>(new app::ATCmdURC("CMD_2", "UURC2:", urc2Callback));
    auto testee3 = std::shared_ptr<app::AT>(new app::ATCmd("CMD_3", "REQ3", "REsp3"));
    auto testee4 = std::shared_ptr<app::AT>(new app::ATCmdOK());
    auto testee5 = std::shared_ptr<app::AT>(new app::ATCmdERROR());

    parser.registerAtCommand(testee1.get());
    parser.registerAtCommand(testee2.get());
    parser.registerAtCommand(testee3.get());
    parser.registerAtCommand(testee4.get());
    parser.registerAtCommand(testee5.get());

    parser.parse();

    CHECK(urc1sock == 1);
    CHECK(urc2sock == 4711);
    CHECK(urc1bytes == 5);
    CHECK(urc2bytes == 128);

    CHECK(true == true);
    TestCaseEnd();
}

int ut_USOSTTest(void)
{
    TestCaseBegin();

    std::condition_variable cv;
    std::mutex cv_m;
    int i = 0;

    static std::string testString = " ";
    static std::string recvString(80, '\x00');
    static auto pos_r = recvString.begin();

    std::function<size_t(uint8_t*, const size_t, std::chrono::milliseconds)> recv =
        [&](uint8_t* data, const size_t length, std::chrono::milliseconds timeout) -> size_t {
            static size_t position = 0;
            size_t i = 0;

            if (testString.length() - position <= 0) {
                Trace(ZONE_INFO, "recv Sleep;\r\n");
                std::unique_lock<std::mutex> lk(cv_m);
                cv.wait_for(lk, timeout, [&] {
                return testString.length() - position > 0;
            });
            }

            for ( ; i < length && position < testString.length(); i++) {
                *data = testString[position++];
            }
            return i;
        };

    std::function<size_t(std::string_view, std::chrono::milliseconds)> send =
        [&](std::string_view in, std::chrono::milliseconds) -> size_t {
            size_t i = 0;
            for ( ; i < in.length() && pos_r != recvString.end(); i++) {
                *pos_r++ = in[i];
            }
            return i;
        };

    app::ATParser parser(recv);

    auto testee1 = std::shared_ptr<app::ATCmdUSOST>(new app::ATCmdUSOST(send));
    auto testee2 = std::shared_ptr<app::AT>(new app::ATCmd("CMD_2", "REQ2", "RESP2"));
    auto testee3 = std::shared_ptr<app::AT>(new app::ATCmd("CMD_3", "REQ3", "REsp3"));
    auto testee4 = std::shared_ptr<app::AT>(new app::ATCmdOK());
    auto testee5 = std::shared_ptr<app::AT>(new app::ATCmdERROR());
    auto testee6 = std::shared_ptr<app::ATCmdUSOWR>(new app::ATCmdUSOWR(send));

    parser.registerAtCommand(std::dynamic_pointer_cast<app::AT>(testee1).get());
    parser.registerAtCommand(std::dynamic_pointer_cast<app::AT>(testee6).get());
    parser.registerAtCommand(testee2.get());
    parser.registerAtCommand(testee3.get());
    parser.registerAtCommand(testee4.get());
    parser.registerAtCommand(testee5.get());

    auto send1 = [&](int j = 2)
                 {
                     Trace(ZONE_INFO, "Sleep %d\r\n", j);
                     {
                         std::unique_lock<std::mutex> lk(cv_m);
                         cv.wait(lk, [&] {
                return j == i;
            });

                         Trace(ZONE_INFO, "Hello %d\r\n", j);
                         testString += "\rOK\r";
                     }
                     auto ret =
                         std::dynamic_pointer_cast<app::ATCmd>(testee3)->send(send, std::chrono::milliseconds(2000));
                     CHECK(ret == app::AT::Return_t::FINISHED);
                 };

    auto send2 = [&](int j = 0)
                 {
                     Trace(ZONE_INFO, "Sleep %d\r\n", j);
                     {
                         std::unique_lock<std::mutex> lk(cv_m);
                         cv.wait(lk, [&] {
                return j == i;
            });
                         testString += "@\rOK\rERROR\r";
                         Trace(ZONE_INFO, "Hello %d\r\n", j);
                     }
                     auto ret = std::dynamic_pointer_cast<app::ATCmdUSOST>(testee1)->send(0,
                                                                                          "ip",
                                                                                          "port",
                                                                                          "hello",
                                                                                          std::chrono::milliseconds(
                                                                                                                    1000));
                     CHECK(ret == app::AT::Return_t::FINISHED);
                 };

    auto parse = [&](int j = 1)
                 {
                     Trace(ZONE_INFO, "Sleep %d\r\n", j);
                     {
                         std::unique_lock<std::mutex> lk(cv_m);
                         cv.wait(lk, [&] {
                return j == i;
            });
                     }
                     Trace(ZONE_INFO, "Hello %d\r\n", j);

                     for (auto i = 0; i < 2; i++) {
                         parser.parse(std::chrono::milliseconds(100));
                     }
                     Trace(ZONE_INFO, "parser END \r\n");
                 };

    auto signals = [&]
                   {
                       Trace(ZONE_INFO, "Signal");

                       std::this_thread::sleep_for(std::chrono::milliseconds(50));
                       {
                           std::lock_guard<std::mutex> lk(cv_m);
                           i = 0;
                       }
                       cv.notify_all();

                       std::this_thread::sleep_for(std::chrono::milliseconds(10));

                       {
                           std::lock_guard<std::mutex> lk(cv_m);
                           i = 1;
                       }
                       cv.notify_all();
                       std::this_thread::sleep_for(std::chrono::milliseconds(100));

                       {
                           std::lock_guard<std::mutex> lk(cv_m);
                           i = 2;
                       }
                       cv.notify_all();
                       std::this_thread::sleep_for(std::chrono::milliseconds(100));

                       {
                           std::lock_guard<std::mutex> lk(cv_m);
                           i = 3;
                       }
                       cv.notify_all();
                       std::this_thread::sleep_for(std::chrono::milliseconds(100));
                   };

    std::thread t0(send2, 1), t1(parse, 0), t2(send1, 2), t3(signals);
    Trace(ZONE_INFO, "Wait \r\n");

    t0.join();
    t1.join();
    t2.join();
    t3.join();

    TestCaseEnd();
}

int ut_TimeoutTest(void)
{
    TestCaseBegin();

    std::condition_variable cv;
    std::mutex cv_m;
    int i = 0;

    static std::string testString = " ";

    std::function<size_t(uint8_t*, const size_t, std::chrono::milliseconds)> recv =
        [&](uint8_t* data, const size_t length, std::chrono::milliseconds timeout) -> size_t {
            static size_t position = 0;
            size_t i = 0;

            if (testString.length() - position <= 0) {
                Trace(ZONE_INFO, "recv Sleep;\r\n");
                std::unique_lock<std::mutex> lk(cv_m);
                cv.wait_for(lk, timeout, [&] {
                return testString.length() - position > 0;
            });
            }

            for ( ; i < length && position < testString.length(); i++) {
                *data = testString[position++];
            }
            return i;
        };

    std::function<size_t(std::string_view, std::chrono::milliseconds)> send =
        [](std::string_view in, std::chrono::milliseconds) -> size_t {
            return in.length();
        };

    app::ATParser parser(recv);

    auto testee1 = std::shared_ptr<app::AT>(new app::ATCmd("CMD_1", "REQ1", "RESP1"));
    auto testee2 = std::shared_ptr<app::AT>(new app::ATCmd("CMD_2", "REQ2", "RESP2"));
    auto testee3 = std::shared_ptr<app::AT>(new app::ATCmd("CMD_3", "REQ3", "REsp3"));
    auto testee4 = std::shared_ptr<app::AT>(new app::ATCmdOK());
    auto testee5 = std::shared_ptr<app::AT>(new app::ATCmdERROR());

    parser.registerAtCommand(testee1.get());
    parser.registerAtCommand(testee2.get());
    parser.registerAtCommand(testee3.get());
    parser.registerAtCommand(testee4.get());
    parser.registerAtCommand(testee5.get());

    auto send1 = [&](int j = 2)
                 {
                     Trace(ZONE_INFO, "Sleep %d\r\n", j);
                     {
                         std::unique_lock<std::mutex> lk(cv_m);
                         cv.wait(lk, [&] {
                return j == i;
            });

                         Trace(ZONE_INFO, "Hello %d\r\n", j);
                     }
                     auto ret =
                         std::dynamic_pointer_cast<app::ATCmd>(testee3)->send(send, std::chrono::milliseconds(2000));
                     CHECK(ret == app::AT::Return_t::TRY_AGAIN);
                 };

    auto send2 = [&](int j = 0)
                 {
                     Trace(ZONE_INFO, "Sleep %d\r\n", j);
                     {
                         std::unique_lock<std::mutex> lk(cv_m);
                         cv.wait(lk, [&] {
                return j == i;
            });
                         Trace(ZONE_INFO, "Hello %d\r\n", j);
                     }
                     auto ret =
                         std::dynamic_pointer_cast<app::ATCmd>(testee2)->send(send, std::chrono::milliseconds(2000));
                     CHECK(ret == app::AT::Return_t::ERROR);
                 };

    auto parse = [&](int j = 1)
                 {
                     Trace(ZONE_INFO, "Sleep %d\r\n", j);
                     {
                         std::unique_lock<std::mutex> lk(cv_m);
                         cv.wait(lk, [&] {
                return j == i;
            });
                     }
                     Trace(ZONE_INFO, "Hello %d\r\n", j);

                     for (auto i = 0; i < 2; i++) {
                         parser.parse(std::chrono::milliseconds(500));
                     }
                     Trace(ZONE_INFO, "parser END \r\n");
                 };

    auto signals = [&]
                   {
                       Trace(ZONE_INFO, "Signal");

                       std::this_thread::sleep_for(std::chrono::milliseconds(50));
                       {
                           std::lock_guard<std::mutex> lk(cv_m);
                           i = 0;
                       }
                       cv.notify_all();

                       std::this_thread::sleep_for(std::chrono::milliseconds(10));

                       {
                           std::lock_guard<std::mutex> lk(cv_m);
                           i = 1;
                       }
                       cv.notify_all();
                       std::this_thread::sleep_for(std::chrono::milliseconds(10));

                       {
                           std::lock_guard<std::mutex> lk(cv_m);
                           i = 2;
                       }
                       cv.notify_all();
                       std::this_thread::sleep_for(std::chrono::milliseconds(10));

                       {
                           std::lock_guard<std::mutex> lk(cv_m);
                           i = 3;
                       }
                       cv.notify_all();
                       std::this_thread::sleep_for(std::chrono::milliseconds(10));
                   };

    std::thread t0(send2, 1), t1(parse, 0), t2(send1, 2), t3(signals);
    Trace(ZONE_INFO, "Wait \r\n");

    t0.join();
    t1.join();
    t2.join();
    t3.join();

    TestCaseEnd();
}

int main(int argc, const char* argv[])
{
    UnitTestMainBegin();
    RunTest(false, ut_BasicTest);
    RunTest(false, ut_ATParserURCTest);
    RunTest(false, ut_USOSTTest);
    RunTest(false, ut_TimeoutTest);

    UnitTestMainEnd();
}
