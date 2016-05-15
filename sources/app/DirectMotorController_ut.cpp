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
#include "DirectMotorController.h"
#include "TaskInterruptable.h"
#include <cmath>
#include <cstdlib>
#include <cstring>

#define NUM_TEST_LOOPS 255

//--------------------------BUFFERS--------------------------
static float g_currentRPS;
static int32_t g_currentPWM;
static bool g_taskJoined;
static bool g_taskStarted;
static uint32_t g_currentTickCount;
static float g_currentOmega;
static float g_batteryVoltage;
static dev::SensorBLDC::Mode g_currentMode;

constexpr const uint32_t POLE_PAIRS = 7;

//--------------------------MOCKING--------------------------

constexpr const std::array<const hal:: Adc::Channel,
                           hal::Adc::Channel::__ENUM__SIZE> hal::Factory<hal::Adc>::ChannelContainer;
constexpr const std::array<const dev::SensorBLDC,
                           dev::SensorBLDC::Description::__ENUM__SIZE> dev::Factory<dev::SensorBLDC>::Container;
constexpr const std::array<const hal::HalfBridge,
                           hal::HalfBridge::Description::__ENUM__SIZE> hal::Factory<hal::HalfBridge>::Container;
constexpr const std::array<const hal::HallDecoder,
                           hal::HallDecoder::Description::__ENUM__SIZE> hal::Factory<hal::HallDecoder>::Container;
constexpr const std::array<const hal::Tim, hal::Tim::Description::__ENUM__SIZE + 1> hal::Factory<hal::Tim>::Container;

//****** Task functions ******
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

//****** SensorBLDC functions ******
float dev::SensorBLDC::getCurrentRPS(void) const
{
    return g_currentRPS;
}

float dev::SensorBLDC::getCurrentOmega(void) const
{
    return g_currentOmega;
}

void dev::SensorBLDC::setPulsWidthInMill(const int32_t value) const
{
    g_currentPWM = value;
}

void dev::SensorBLDC::setMode(dev::SensorBLDC::Mode mode) const
{
    g_currentMode = mode;
}

int32_t dev::SensorBLDC::getPulsWidthPerMill(void) const
{
    return g_currentPWM;
}

dev::SensorBLDC::Direction dev::SensorBLDC::getDirection() const
{
    if (g_currentOmega > 0) {
        return dev::SensorBLDC::Direction::BACKWARD;
    } else {
        return dev::SensorBLDC::Direction::FORWARD;
    }
}

void dev::SensorBLDC::checkMotor(const dev::Battery&) const
{
    //TODO Test ME
}

void dev::SensorBLDC::start() const
{
    //TODO Test ME
}

void dev::SensorBLDC::stop() const
{
    //TODO Test ME
}

uint32_t dev::SensorBLDC::getNumberOfPolePairs() const
{
    return POLE_PAIRS;
}

//****** Battery functions ******
dev::Battery::Battery(){}

dev::Battery::~Battery(){}

float dev::Battery::getVoltage(void) const
{
    return g_batteryVoltage;
}

float dev::Battery::getCurrent(void) const
{
    return 0;
}

float dev::Battery::getPower(void) const
{
    return 0;
}

float dev::Battery::getTemperature(void) const
{
    return 0.0;
}

void vQueueDelete(QueueHandle_t xQueue) {}

void* queueBuffer = nullptr;
size_t queueElementSize = 0;
size_t queueBufferSize = 0;

QueueHandle_t xQueueGenericCreate(const UBaseType_t uxQueueLength,
                                  const UBaseType_t uxItemSize,
                                  const uint8_t     ucQueueType)
{
    queueBuffer = std::malloc(uxItemSize * uxQueueLength);
    queueBufferSize = uxItemSize * uxQueueLength;
    queueElementSize = uxItemSize;
    return queueBuffer;
}

BaseType_t xQueueGenericReceive(QueueHandle_t    xQueue,
                                void* const      pvBuffer,
                                TickType_t       xTicksToWait,
                                const BaseType_t xJustPeek)
{
    if (queueBuffer == nullptr) {return 0; }
    std::memcpy(pvBuffer, queueBuffer, queueElementSize);
    return 1;
}

BaseType_t xQueueGenericSend(QueueHandle_t     xQueue,
                             const void* const pvItemToQueue,
                             TickType_t        xTicksToWait,
                             const BaseType_t  xCopyPosition)
{
    if (queueBuffer == nullptr) {return 0; }
    std::memcpy(queueBuffer, pvItemToQueue, queueElementSize);
    return 1;
}

//-------------------------HELPERS-------------------------
template<size_t n>
void plotLines(std::array<std::pair<float, float>, n> line1, std::array<std::pair<float, float>, n> line2)
{
#if 0
    FILE* gnuplotPipe = popen("gnuplot -persistent", "w");

    fprintf(gnuplotPipe, "set style line 1 lc rgb '#0060ad' lt 1 lw 2 pt 0 ps 1 \n");
    fprintf(gnuplotPipe, "set style line 2 lc rgb '#ad6000' lt 1 lw 2 pt 0 ps 1 \n");

    fprintf(gnuplotPipe, "set multiplot \n");
    fprintf(gnuplotPipe, "set size 1, 0.5 \n");
    fprintf(gnuplotPipe, "set origin 0.0,0.5 \n");

    fprintf(gnuplotPipe, "plot '-' with linespoints ls 1 \n");
    for (auto pair : line1) {
        fprintf(gnuplotPipe, "%lf %lf\n", pair.first, pair.second);
    }
    fprintf(gnuplotPipe, "e \n");
    fprintf(gnuplotPipe, "set origin 0.0,0.0 \n");

    fprintf(gnuplotPipe, "plot '-' with linespoints ls 2 \n");
    for (auto pair : line2) {
        fprintf(gnuplotPipe, "%lf %lf\n", pair.first, pair.second);
    }
    fprintf(gnuplotPipe, "e \n");
    fprintf(gnuplotPipe, "unset multiplot \n");
#endif
}

//-------------------------TESTCASES-------------------------

int ut_TestSetTorque(void)
{
    TestCaseBegin();

    app::DirectMotorController testee(dev::Factory<dev::SensorBLDC>::get<dev::SensorBLDC::BLDC>(),
                                      dev::Battery(), 1.0, 1.0, 1.0);
    testee.setTorque(1);

    CHECK(true);

    TestCaseEnd();
}

int ut_TestOmegaRamp(void)
{
    TestCaseBegin();

    static constexpr const auto NUMBER_OF_VALUES = 500;

    std::array<std::pair<float, float>, NUMBER_OF_VALUES> omega;
    std::array<std::pair<float, float>, NUMBER_OF_VALUES> pwm;

    size_t counter = 0;
    for (auto& pair : omega) {
        pair.first = counter;
        pair.second = counter++;
    }

    counter = 0;
    for (auto& pair : pwm) {
        pair.first = counter++;
        pair.second = 0;
    }

    app::DirectMotorController testee(dev::Factory<dev::SensorBLDC>::get<dev::SensorBLDC::BLDC>(),
                                      dev::Battery(), 0.0749, 0.795, 0.000261);
    testee.setTorque(0.5);
    g_batteryVoltage = 40.0;

    for (int i = 0; i < NUMBER_OF_VALUES; i++) {
        g_currentOmega = omega[i].second;
        testee.triggerTaskExecution();
        pwm[i].second = g_currentPWM;
    }

//    for (auto pair : pwm) {
//        CHECK(pair.second<0.01 && pair.second> -0.01);
//    }

    plotLines(omega, pwm);

    TestCaseEnd();
}

int main(int argc, const char* argv[])
{
    UnitTestMainBegin();
    RunTest(true, ut_TestSetTorque);
    RunTest(true, ut_TestOmegaRamp);
    UnitTestMainEnd();
}
