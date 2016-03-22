/* Copyright (C) 2015  Nils Weiss, Alexander Strobl
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
#include "MotorController.h"
#include "TaskInterruptable.h"
#include "PIDController.h"
#include <cmath>

#define NUM_TEST_LOOPS 255

//--------------------------BUFFERS--------------------------
static float g_currentRPS;
static int32_t g_currentPWM;
static bool g_taskJoined;
static bool g_taskStarted;
static uint32_t g_currentTickCount;
static float g_currentOmega;
static float g_batteryVoltage;

static float g_PID_P;
static float g_PID_I;
static float g_PID_D;

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

void dev::SensorBLDC::checkMotor() const
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

//****** Battery functions ******
dev::Battery::Battery(){}

dev::Battery::~Battery(){}

float dev::Battery::getVoltage(void) const
{
    return 0;
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

//***** PID Controller *****
dev::PIDController::PIDController(float&                 input,
                                  float&                 output,
                                  float&                 setPoint,
                                  const float            kp,
                                  const float            ki,
                                  const float            kd,
                                  const ControlDirection direction) :
    mInput(input),
    mOutput(output),
    mSetPoint(setPoint)
{
    g_PID_D = kd;
    g_PID_I = ki;
    g_PID_P = kp;
}

bool dev::PIDController::compute(void)
{
//    mOutput = 1;
    return true;
}

void dev::PIDController::setSampleTime(const std::chrono::milliseconds newSampleTime)
{}

void dev::PIDController::setOutputLimits(const float min, const float max)
{}

void dev::PIDController::setMode(const ControlMode newMode)
{}

void vQueueDelete(QueueHandle_t xQueue) {}

QueueHandle_t xQueueGenericCreate(const UBaseType_t uxQueueLength,
                                  const UBaseType_t uxItemSize,
                                  const uint8_t     ucQueueType)
{
    return 0;
}

BaseType_t xQueueGenericReceive(QueueHandle_t    xQueue,
                                void* const      pvBuffer,
                                TickType_t       xTicksToWait,
                                const BaseType_t xJustPeek)
{
    return 0;
}

BaseType_t xQueueGenericSend(QueueHandle_t     xQueue,
                             const void* const pvItemToQueue,
                             TickType_t        xTicksToWait,
                             const BaseType_t  xCopyPosition)
{
    return 0;
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

int ut_TestConstructor(void)
{
    TestCaseBegin();

    app::MotorController testee(dev::Factory<dev::SensorBLDC>::get<dev::SensorBLDC::BLDC>(),
                                dev::Battery(), 0.00768, 0.8, 0.6, 0.1);

    CHECK(g_PID_D == 0);
    CHECK(g_PID_P <= 0.601 && g_PID_P >= 0.599);
    CHECK(g_PID_I <= 0.101 && g_PID_I >= 0.099);

    TestCaseEnd();
}

int ut_TestSetTorque(void)
{
    TestCaseBegin();

    app::MotorController testee(dev::Factory<dev::SensorBLDC>::get<dev::SensorBLDC::BLDC>(),
                                dev::Battery(), 0.00768, 0.8, 0.6, 0.1);
    testee.setTorque(1);

    CHECK(true);

    TestCaseEnd();
}

int ut_TestPIDInput(void)
{
    TestCaseBegin();

    app::MotorController testee(dev::Factory<dev::SensorBLDC>::get<dev::SensorBLDC::BLDC>(),
                                dev::Battery(), 0.00768, 0.8, 0.6, 0.1);

    std::array<std::pair<float, float>, 1000> omegas;
    std::array<std::pair<float, float>, 1000> torques;

    auto counter = 1;
    for (auto& pair : omegas) {
        pair.first = counter - 500;
        pair.second = counter++ - 500;
    }

    counter = 0;
    for (const auto& pair : omegas) {
        // set input values
        g_batteryVoltage = 36;
        g_currentPWM = 1000;
        g_currentOmega = pair.second;

        // execute Task
        testee.triggerTaskExecution();

        // save output
        torques[counter].first = counter + 1 - 500;
        torques[counter].second = testee.mCurrentTorque;
        counter++;
    }

    plotLines(omegas, torques);
    CHECK(true);

    TestCaseEnd();
}

int ut_TestPIDOutput(void)
{
    TestCaseBegin();

    app::MotorController testee(dev::Factory<dev::SensorBLDC>::get<dev::SensorBLDC::BLDC>(),
                                dev::Battery(), 0.00768, 0.8, 0.6, 0.1);

    std::array<std::pair<float, float>, 1000> torque;
    std::array<std::pair<float, float>, 1000> pwms;

    auto counter = 1;
    for (auto& pair : torque) {
        pair.first = counter - 500;
        pair.second = (counter++ - 500) * 0.0001;
    }

    counter = 0;
    for (const auto& pair : torque) {
        // set input values
        g_batteryVoltage = 36;
        g_currentOmega = 500;
        g_currentPWM = 1000;
        testee.mOutputTorque = pair.second;

        // execute Task
        testee.triggerTaskExecution();

        // save output
        pwms[counter].first = counter + 1 - 500;
        pwms[counter++].second = g_currentPWM;
    }

    plotLines(torque, pwms);
    CHECK(true);

    TestCaseEnd();
}

int ut_TestPIDInOut(void)
{
    TestCaseBegin();

    app::MotorController testee(dev::Factory<dev::SensorBLDC>::get<dev::SensorBLDC::BLDC>(),
                                dev::Battery(), 0.00768, 0.8, 0.6, 0.1);

    std::array<std::pair<float, float>, 1000> omegas;
    std::array<std::pair<float, float>, 1000> pwms;

    auto offset = -500;

    auto counter = 1;
    for (auto& pair : omegas) {
        pair.first = counter + offset;
        pair.second = (counter++ + offset);
    }

    counter = 0;
    for (const auto& pair : omegas) {
        // set input values
        g_batteryVoltage = 36;
        g_currentPWM = 100;
        g_currentOmega = pair.second;

        testee.mOutputTorque = 0.01;

        // execute Task
        testee.triggerTaskExecution();

        // save output
        pwms[counter].first = counter + 1 + offset;
        pwms[counter++].second = g_currentPWM;
    }

    plotLines(omegas, pwms);
    CHECK(true);

    TestCaseEnd();
}

int main(int argc, const char* argv[])
{
    UnitTestMainBegin();
    RunTest(true, ut_TestConstructor);
    RunTest(true, ut_TestSetTorque);
    RunTest(false, ut_TestPIDInput);
    RunTest(false, ut_TestPIDOutput);
    RunTest(false, ut_TestPIDInOut);
    UnitTestMainEnd();
}
