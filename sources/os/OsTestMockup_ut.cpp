/// @file OsTestMockup_ut.cpp
/// @brief Test cases for the os TestMockups on which other tests rely.
/// @author Henning Mende (henning@my-urmo.com)
/// @date   Nov 19, 2019
/// @copyright UrmO GmbH
///
/// You need to be sure your testing tools work!
///
/// This program is free software: you can redistribute it and/or modify it under the terms
/// of the GNU General Public License as published by the Free Software Foundation, either
/// version 3 of the License, or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
/// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
/// See the GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License along with this program.
/// If not, see <https://www.gnu.org/licenses/>.
#include "unittest.h"

#include "os_Task.h"
#include "Semaphore.h"
#include "Mutex.h"
#include "LockGuard.h"

int ut_TestTaskMock(void)
{
    TestCaseBegin();

    constexpr int want = 100;
    int have = 0;

    {
        os::Task sut("name", 1024, os::Task::Priority::MEDIUM, [&have](const bool& join){
                     for (unsigned int i = 0; i < 100; i++) {
                         os::ThisTask::yield();
                         have++;
                     }
                     os::ThisTask::sleep(std::chrono::milliseconds(5));
            });

        CHECK(want != have);
    }

    CHECK(want == have);

    TestCaseEnd();
}

int ut_TestMutexMock(void)
{
    TestCaseBegin();
    constexpr unsigned int compareValue = 1000, want = compareValue * 2;
    unsigned int have = 0;
    const os::Mutex mutex;

    std::function<void(const bool&)> funcWithout([&](const bool& join) {
                                                 for (unsigned int i = 0; i < compareValue; i++) {
                                                     const unsigned int tmp = have;
                                                     os::ThisTask::yield();
                                                     have = tmp + 1;
                                                     os::ThisTask::yield();
                                                 }
        });

    std::function<void(const bool&)> funcWith([&](const bool& join) {
                                              for (unsigned int i = 0; i < compareValue; i++) {
                                                  mutex.take();
                                                  const unsigned int tmp = have;
                                                  os::ThisTask::yield();
                                                  have = tmp + 1;
                                                  mutex.give();
                                                  os::ThisTask::yield();
                                              }
        });

    {
        os::Task t1("t1", 1, os::Task::Priority::MEDIUM, funcWithout);
        os::Task t2("t2", 2, os::Task::Priority::MEDIUM, funcWithout);
    }

    CHECK(want != have);

    {
        have = 0;
        os::Task t1("t1", 1, os::Task::Priority::MEDIUM, funcWith);
        os::Task t2("t2", 2, os::Task::Priority::MEDIUM, funcWith);
    }

    CHECK(want == have);

    TestCaseEnd();
}

int ut_TestSemaphoreMock(void)
{
    TestCaseBegin();

    constexpr unsigned int compareValue = 1000, want = compareValue + 1;
    unsigned int have = 0;
    bool done = false;
    const os::Semaphore semaphore;

    std::function<void(const bool&)> supplier([&](const bool& join) {
                                              for (unsigned int i = 0; i < compareValue; i++) {
                                                  semaphore.give();
                                                  os::ThisTask::yield();
                                                  os::ThisTask::sleep(std::chrono::milliseconds(1));
                                              }
                                              done = true;
                                              semaphore.give();
        });

    std::function<void(const bool&)> consumer([&](const bool& join) {
                                              while (!done) {
                                                  semaphore.take();
                                                  have++;
                                              }
        });

    {
        os::Task consumerTask("t1", 1, os::Task::Priority::MEDIUM, consumer);
        os::Task supplierTask("t2", 2, os::Task::Priority::MEDIUM, supplier);
    }

    CHECK(want == have);

    TestCaseEnd();
}

int main(int argc, char** argv)
{
    UnitTestMainBegin();
    RunTest(true, ut_TestTaskMock);
    RunTest(true, ut_TestMutexMock);
    RunTest(true, ut_TestSemaphoreMock);
    UnitTestMainEnd();
}
