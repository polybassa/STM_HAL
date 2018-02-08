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

#include "unittest.h"
#include "AT_Cmd.h"
#include "AT_Parser.h"
#include <condition_variable>
#include <thread>
#include <mutex>

#define NUM_TEST_LOOPS 255

//--------------------------BUFFERS--------------------------

//--------------------------MOCKING--------------------------

class semaphoreMock
{
    std::condition_variable cv;
    std::mutex cv_m;
    int i = 0;
public:
    void give()
    {
        i = 1;
        cv.notify_all();
    }
    void take()
    {
        std::unique_lock<std::mutex> lk(cv_m);
        cv.wait(lk, [&](){return i == 1;
                });
    }
};

void os::ThisTask::sleep(const std::chrono::milliseconds ms) {}
os::Semaphore::Semaphore(void) :
    mSemaphoreHandle(reinterpret_cast<void*>(new semaphoreMock))
{}
os::Semaphore::~Semaphore(void)
{
    auto x = reinterpret_cast<semaphoreMock*>(mSemaphoreHandle);
    delete x;
}
bool os::Semaphore::give(void) const
{
    auto x = reinterpret_cast<semaphoreMock*>(mSemaphoreHandle);
    x->give();
    return true;
}
bool os::Semaphore::take(uint32_t ticksToWait) const
{
    auto x = reinterpret_cast<semaphoreMock*>(mSemaphoreHandle);

    x->take();
    return true;
}

//-------------------------TESTCASES-------------------------

int ut_ATParserBasicTest(void)
{
    TestCaseBegin();

    static std::string testString = "\rRESP2\rOK\rERROR\rOK\r\nPES";
    static auto pos = testString.begin();

    std::function<size_t(uint8_t*, const size_t, std::chrono::milliseconds)> recv =
    [&](uint8_t * data, const size_t length, std::chrono::milliseconds)->size_t {
        size_t i = 0;
        for ( ; i < length && pos != testString.end(); i++) {
            *data = *pos++;
        }
        return i;
    };

    std::function<size_t(std::string_view, std::chrono::milliseconds)> send =
        [] (std::string_view in, std::chrono::milliseconds)->size_t {
        return in.length();
    };

    app::ATParser parser(recv);

    auto testee1 = std::shared_ptr<app::AT>(new app::ATCmd("CMD_1", "REQ1", "RESP1", parser));
    auto testee2 = std::shared_ptr<app::AT>(new app::ATCmd("CMD_2", "REQ2", "RESP2", parser));
    auto testee3 = std::shared_ptr<app::AT>(new app::ATCmd("CMD_3", "REQ3", "REsp3", parser));
    auto testee4 = std::shared_ptr<app::AT>(new app::ATCmdOK(parser));
    auto testee5 = std::shared_ptr<app::AT>(new app::ATCmdERROR(parser));

    parser.registerAtCommand(testee1);
    parser.registerAtCommand(testee2);
    parser.registerAtCommand(testee3);
    parser.registerAtCommand(testee4);
    parser.registerAtCommand(testee5);

    auto ret = std::dynamic_pointer_cast<app::ATCmd>(testee2)->send(send);

    CHECK(ret == app::AT::ReturnType::WAITING);

    ret = std::dynamic_pointer_cast<app::ATCmd>(testee3)->send(send);

    CHECK(ret == app::AT::ReturnType::TRY_AGAIN);

    CHECK(testee2->isCommandFinished() == false)

    parser.parse();

    CHECK(testee2->isCommandFinished() == true)

    ret = std::dynamic_pointer_cast<app::ATCmd>(testee3)->send(send);

    CHECK(ret == app::AT::ReturnType::WAITING);

    CHECK(true == true);
    TestCaseEnd();
}

int ut_ATParserURCTest(void)
{
    TestCaseBegin();

    static std::string testString = "\rRESP2\rOK\rERROR\rUURC1: 1,5\rOK\r\nPESUURC2: 4711,128\r\nOK";
    static auto pos = testString.begin();

    std::function<size_t(uint8_t*, const size_t, std::chrono::milliseconds)> recv =
    [&](uint8_t * data, const size_t length, std::chrono::milliseconds)->size_t {
        size_t i = 0;
        for ( ; i < length && pos != testString.end(); i++) {
            *data = *pos++;
        }
        return i;
    };

    std::function<size_t(std::string_view, std::chrono::milliseconds)> send =
        [] (std::string_view in, std::chrono::milliseconds)->size_t {
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

    auto testee1 = std::shared_ptr<app::AT>(new app::ATCmdURC("CMD_1", "UURC1:", parser, urc1Callback));
    auto testee2 = std::shared_ptr<app::AT>(new app::ATCmdURC("CMD_2", "UURC2:", parser, urc2Callback));
    auto testee3 = std::shared_ptr<app::AT>(new app::ATCmd("CMD_3", "REQ3", "REsp3", parser));
    auto testee4 = std::shared_ptr<app::AT>(new app::ATCmdOK(parser));
    auto testee5 = std::shared_ptr<app::AT>(new app::ATCmdERROR(parser));

    parser.registerAtCommand(testee1);
    parser.registerAtCommand(testee2);
    parser.registerAtCommand(testee3);
    parser.registerAtCommand(testee4);
    parser.registerAtCommand(testee5);

    parser.parse();

    auto ret = std::dynamic_pointer_cast<app::ATCmd>(testee3)->send(send);
    CHECK(ret == app::AT::ReturnType::WAITING);

    CHECK(urc1sock == 1);
    CHECK(urc2sock == 4711);
    CHECK(urc1bytes == 5);
    CHECK(urc2bytes == 128);

    CHECK(true == true);
    TestCaseEnd();
}

int ut_ATParserUSOSTTest(void)
{
    TestCaseBegin();

    static std::string testString = "@\rOK\rERROR\r";

    static std::string recvString(80, '\x00');
    static auto pos_r = recvString.begin();

    std::function<size_t(uint8_t*, const size_t, std::chrono::milliseconds)> recv =
    [&](uint8_t * data, const size_t length, std::chrono::milliseconds)->size_t {
        static auto pos = testString.begin();
        size_t i = 0;
        for ( ; i < length && pos != testString.end(); i++) {
            *data = *pos++;
        }
        return i;
    };

    std::function<size_t(std::string_view, std::chrono::milliseconds)> send =
    [&](std::string_view in, std::chrono::milliseconds)->size_t {
        size_t i = 0;
        for ( ; i < in.length() && pos_r != recvString.end(); i++) {
            *pos_r++ = in[i];
        }
        return i;
    };

    app::ATParser parser(recv);

    auto testee1 = std::shared_ptr<app::ATCmdUSOST>(new app::ATCmdUSOST(send, parser));
    auto testee3 = std::shared_ptr<app::AT>(new app::ATCmd("CMD_3", "REQ3", "REsp3", parser));
    auto testee4 = std::shared_ptr<app::AT>(new app::ATCmdOK(parser));
    auto testee5 = std::shared_ptr<app::AT>(new app::ATCmdERROR(parser));

    parser.registerAtCommand(std::dynamic_pointer_cast<app::AT>(testee1));
    parser.registerAtCommand(testee3);
    parser.registerAtCommand(testee4);
    parser.registerAtCommand(testee5);

    std::thread t1 = std::thread([&]
                                 {
                                     auto ret =
                                         std::dynamic_pointer_cast<app::ATCmdUSOST>(testee1)->send("hello",
                                                                                                   std::chrono::
                                                                                                   milliseconds(1000));
                                     CHECK(ret == app::AT::ReturnType::FINISHED);
                                 });

    std::thread t2 = std::thread([&] {
                                     os::ThisTask::sleep(std::chrono::milliseconds(1000));

                                     parser.parse();
                                 });

    t2.join();
    t1.join();

    CHECK(recvString.find("hello") != std::string::npos);

    TestCaseEnd();
}

int main(int argc, const char* argv[])
{
    UnitTestMainBegin();
    RunTest(true, ut_ATParserBasicTest);
    RunTest(true, ut_ATParserURCTest);
    RunTest(true, ut_ATParserUSOSTTest);
    UnitTestMainEnd();
}
