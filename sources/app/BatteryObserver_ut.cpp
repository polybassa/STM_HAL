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

#include "unittest.h"
#include "BatteryObserver.h"
#include "TaskInterruptable.h"
#include <cmath>

#define NUM_TEST_LOOPS 255

using std::chrono::duration_cast;
using std::chrono::hours;
using std::chrono::milliseconds;

//--------------------------BUFFERS--------------------------
static float g_currentPower;
static uint32_t g_currentTickCount;
static hal::Rtc::time_point g_systemTimeNow;
static float g_currentVoltage;
static float g_currentCurrent;
static bool g_taskJoined, g_taskStarted;

//--------------------------MOCKING--------------------------

constexpr const std::array<const hal::Adc::Channel,
                           hal::Adc::Channel::__ENUM__SIZE> hal::Factory<hal::Adc>::ChannelContainer;

// Task functions
void os::TaskInterruptable::join(void)
{
    g_taskJoined = true;
}

void os::TaskInterruptable::start(void)
{
    g_taskStarted = true;
}

os::TaskInterruptable::TaskInterruptable(char const* name, unsigned short stack, unsigned int prio,
                                         std::function<void(bool const&)> func) : Task(name, stack, prio, func) {}

os::TaskInterruptable::~TaskInterruptable(void) {}

void os::TaskInterruptable::taskFunction(void) {}

os::Task::Task(char const* name, unsigned short stack, unsigned int prio, std::function<void(bool const&)> func) {}

os::Task::~Task(void) {}

void os::Task::taskFunction(void) {}

void os::ThisTask::sleep(const std::chrono::milliseconds ms) {}

uint32_t os::Task::getTickCount(void)
{
    return g_currentTickCount;
}

hal::Rtc::time_point hal::Rtc::now(void) noexcept
{
    return g_systemTimeNow;
}

float dev::Battery::getVoltage(void) const
{
    return g_currentVoltage;
}

float dev::Battery::getCurrent(void) const
{
    return g_currentCurrent;
}

float dev::Battery::getPower(void) const
{
    return g_currentPower;
}

hal::Rtc::time_point hal::Rtc::from_time_t(std::time_t __t) noexcept
{
    return std::chrono::time_point_cast<hal::Rtc::duration>(time_point(std::chrono::seconds(__t)));
}

//-------------------------TESTCASES-------------------------

int ut_LoadBattery(void)
{
    TestCaseBegin();

    g_currentCurrent = g_currentPower = 0;
    g_currentVoltage = 10;
    g_currentTickCount = 0;

    dev::Battery batt;

    app::BatteryObserver testee(batt, [](app::BatteryObserver::ErrorCode error) {
        printf("Error %d\n", error);
    });

    testee.triggerTaskExecution();

    g_currentPower = 1;
    g_currentTickCount = duration_cast<milliseconds>(hours(2)).count();

    testee.triggerTaskExecution();

    CHECK(100 == testee.getEnergyLevelInPercent());
    CHECK(2 == testee.getMaxEnergy());
    CHECK(2 == testee.getEnergy());

    g_currentTickCount = duration_cast<milliseconds>(hours(4)).count();

    testee.triggerTaskExecution();

    CHECK(100 == testee.getEnergyLevelInPercent());
    CHECK(4 == testee.getMaxEnergy());
    CHECK(4 == testee.getEnergy());

    TestCaseEnd();
}

int ut_LoadAndDischargeBattery(void)
{
    TestCaseBegin();

    g_currentCurrent = g_currentPower = 0;
    g_currentVoltage = 10;
    g_currentTickCount = 0;

    dev::Battery batt;

    app::BatteryObserver testee(batt, [](app::BatteryObserver::ErrorCode error) {
        printf("Error %d\n", error);
    });

    testee.triggerTaskExecution();

    // LOAD to 4 Wh
    g_currentPower = 1;
    g_currentTickCount = duration_cast<milliseconds>(hours(4)).count();

    testee.triggerTaskExecution();

    CHECK(100 == testee.getEnergyLevelInPercent());
    CHECK(4 == testee.getMaxEnergy());
    CHECK(4 == testee.getEnergy());

    // Discharge to 2 Wh
    g_currentPower = -2;
    g_currentTickCount += duration_cast<milliseconds>(hours(1)).count();

    testee.triggerTaskExecution();

    CHECK(50 == testee.getEnergyLevelInPercent());
    CHECK(4 == testee.getMaxEnergy());
    CHECK(2 == testee.getEnergy());

    // Discharge to 2 Wh
    g_currentPower = -2;
    g_currentTickCount += duration_cast<milliseconds>(hours(1)).count();

    testee.triggerTaskExecution();

    CHECK(0 == testee.getEnergyLevelInPercent());
    CHECK(4 == testee.getMaxEnergy());
    CHECK(0 == testee.getEnergy());

    // Discharge to 2 Wh
    g_currentPower = -2;
    g_currentTickCount += duration_cast<milliseconds>(hours(1)).count();

    testee.triggerTaskExecution();

    CHECK(0 == testee.getEnergyLevelInPercent());
    CHECK(4 == testee.getMaxEnergy());
    CHECK(0 == testee.getEnergy());

    TestCaseEnd();
}

int ut_OvercurrentTest(void)
{
    TestCaseBegin();

    g_currentCurrent = g_currentPower = 0;
    g_currentVoltage = 10;
    g_currentTickCount = 0;

    bool overcurrentDetected = false;

    dev::Battery batt;

    app::BatteryObserver testee(batt, [&](app::BatteryObserver::ErrorCode error) {
        if (error == app::BatteryObserver::ErrorCode::OVERCURRENT) {
            overcurrentDetected = true;
        }
    });

    g_currentCurrent = 0;
    testee.triggerTaskExecution();

    CHECK(overcurrentDetected == false);

    g_currentCurrent = 100000;
    testee.triggerTaskExecution();

    CHECK(overcurrentDetected == true);

    TestCaseEnd();
}

int ut_UndervoltageTest(void)
{
    TestCaseBegin();

    g_currentCurrent = g_currentPower = 0;
    g_currentTickCount = 0;

    bool undervoltage = false;

    dev::Battery batt;

    app::BatteryObserver testee(batt, [&](app::BatteryObserver::ErrorCode error) {
        if (error == app::BatteryObserver::ErrorCode::UNDERVOLTAGE) {
            undervoltage = true;
        }
    });

    g_currentVoltage = 1000;
    testee.triggerTaskExecution();

    CHECK(undervoltage == false);

    g_currentVoltage = 0;
    testee.triggerTaskExecution();

    CHECK(undervoltage == true);

    TestCaseEnd();
}

int ut_DeepSleep(void)
{
    TestCaseBegin();

    g_currentCurrent = g_currentPower = 0;
    g_currentVoltage = 10;
    g_currentTickCount = 0;

    dev::Battery batt;

    app::BatteryObserver testee(batt, [](app::BatteryObserver::ErrorCode error) {
        printf("Error %d\n", error);
    });

    testee.triggerTaskExecution();

    g_currentPower = 1;
    g_currentTickCount = duration_cast<milliseconds>(hours(2)).count();

    testee.triggerTaskExecution();

    CHECK(100 == testee.getEnergyLevelInPercent());
    CHECK(2 == testee.getMaxEnergy());
    CHECK(2 == testee.getEnergy());

    g_currentTickCount = duration_cast<milliseconds>(hours(4)).count();

    testee.triggerTaskExecution();

    CHECK(100 == testee.getEnergyLevelInPercent());
    CHECK(4 == testee.getMaxEnergy());
    CHECK(4 == testee.getEnergy());

    // Load to 4 Wh
    g_systemTimeNow = hal::Rtc::from_time_t(0);
    g_taskJoined = g_taskStarted = false;

    os::DeepSleepController::enterGlobalDeepSleep();

    CHECK(true == g_taskJoined);

    // sleep for 1 hour
    g_systemTimeNow = hal::Rtc::from_time_t(3600);

    os::DeepSleepController::exitGlobalDeepSleep();

    CHECK(true == g_taskStarted);
    CHECK(4 == testee.getMaxEnergy());
    CHECK(std::fabs(3.9 - testee.getEnergy()) < std::numeric_limits<float>::epsilon());

    TestCaseEnd();
}

int main(int argc, const char* argv[])
{
    UnitTestMainBegin();
    RunTest(true, ut_LoadBattery);
    RunTest(true, ut_LoadAndDischargeBattery);
    RunTest(true, ut_OvercurrentTest);
    RunTest(true, ut_UndervoltageTest);
    RunTest(true, ut_DeepSleep);
    UnitTestMainEnd();
}
