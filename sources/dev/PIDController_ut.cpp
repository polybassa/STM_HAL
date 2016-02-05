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
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include "PIDController.h"
#include "os_Task.h"
#include <array>
#include <utility>

//--------------------------BUFFERS--------------------------

uint32_t g_Ticks = 0;

//--------------------------MOCKING--------------------------

uint32_t os::Task::getTickCount(void)
{
    return g_Ticks;
}

//-------------------------TESTCASES-------------------------

using dev::PIDController;

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

int ut_TestPIDNoChange(void)
{
    TestCaseBegin();

    static constexpr const auto NUMBER_OF_VALUES = 100;
    static constexpr const auto JUMP_AT = 50;

    std::array<std::pair<float, float>, NUMBER_OF_VALUES> line1;
    std::array<std::pair<float, float>, NUMBER_OF_VALUES> line2;

    size_t counter = 0;
    for (auto& pair : line1) {
        pair.first = counter;
        if (counter++ < JUMP_AT) {
            pair.second = 0;
        } else {
            pair.second = 1;
        }
    }

    counter = 0;
    for (auto& pair : line2) {
        pair.first = counter++;
        pair.second = 0;
    }

    float output;
    float input;
    float setPoint;

    dev::PIDController pid(input, output, setPoint, 10, 0, 0, dev::PIDController::ControlDirection::DIRECT);
    pid.setSampleTime(std::chrono::milliseconds(1));
    pid.setMode(dev::PIDController::ControlMode::AUTOMATIC);

    for (int i = 0; i < NUMBER_OF_VALUES; i++) {
        input = line1[i].second;
        setPoint = line1[i].second;
        g_Ticks += std::chrono::milliseconds(1).count();
        pid.compute();
        line2[i].second = output;
    }

    for (auto pair : line2) {
        CHECK(pair.second == 0);
    }

    plotLines(line1, line2);

    TestCaseEnd();
}

int ut_TestPID_P_Change(void)
{
    TestCaseBegin();

    static constexpr const auto NUMBER_OF_VALUES = 100;
    static constexpr const auto JUMP_AT = 50;

    std::array<std::pair<float, float>, NUMBER_OF_VALUES> line1;
    std::array<std::pair<float, float>, NUMBER_OF_VALUES> line2;

    size_t counter = 0;
    for (auto& pair : line1) {
        pair.first = counter;
        if (counter++ < JUMP_AT) {
            pair.second = 0;
        } else {
            pair.second = 1;
        }
    }

    counter = 0;
    for (auto& pair : line2) {
        pair.first = counter++;
        pair.second = 0;
    }

    float output;
    float input;
    float setPoint;

    dev::PIDController pid(input, output, setPoint, 0.5, 0, 0, dev::PIDController::ControlDirection::DIRECT);
    pid.setSampleTime(std::chrono::milliseconds(1));
    pid.setMode(dev::PIDController::ControlMode::AUTOMATIC);

    for (int i = 0; i < NUMBER_OF_VALUES; i++) {
        input = line1[i].second;
        setPoint = 1;
        g_Ticks += std::chrono::milliseconds(1).count();
        pid.compute();
        line2[i].second = output;
    }

    counter = 0;
    for (auto pair : line2) {
        if (counter++ < JUMP_AT) {
            CHECK(pair.second <= 0.51 && pair.second >= 0.49);
        } else {
            CHECK(pair.second <= 0.01 && pair.second >= -0.01);
        }
    }

    plotLines(line1, line2);

    TestCaseEnd();
}

int ut_TestPID_I_Change(void)
{
    TestCaseBegin();

    static constexpr const auto NUMBER_OF_VALUES = 100;
    static constexpr const auto JUMP_AT = 50;

    std::array<std::pair<float, float>, NUMBER_OF_VALUES> line1;
    std::array<std::pair<float, float>, NUMBER_OF_VALUES> line2;

    size_t counter = 0;
    for (auto& pair : line1) {
        pair.first = counter;
        if (counter++ < JUMP_AT) {
            pair.second = 0;
        } else {
            pair.second = 1;
        }
    }

    counter = 0;
    for (auto& pair : line2) {
        pair.first = counter++;
        pair.second = 0;
    }

    float output;
    float input;
    float setPoint;

    dev::PIDController pid(input, output, setPoint, 0, 10, 0, dev::PIDController::ControlDirection::DIRECT);
    pid.setSampleTime(std::chrono::milliseconds(1));
    pid.setMode(dev::PIDController::ControlMode::AUTOMATIC);

    for (int i = 0; i < NUMBER_OF_VALUES; i++) {
        input = line1[i].second;
        setPoint = 1;
        g_Ticks += std::chrono::milliseconds(1).count();
        pid.compute();
        line2[i].second = output;
    }

    counter = 0;
    for (auto pair : line2) {
        if (counter++ > JUMP_AT) {
            CHECK(pair.second <= 0.51 && pair.second >= 0.49);
        } else {
            CHECK(pair.second >= -0.01);
        }
    }

    plotLines(line1, line2);

    TestCaseEnd();
}
int ut_TestPID_D_Change(void)
{
    TestCaseBegin();

    static constexpr const auto NUMBER_OF_VALUES = 100;
    static constexpr const auto JUMP_AT = 50;

    std::array<std::pair<float, float>, NUMBER_OF_VALUES> line1;
    std::array<std::pair<float, float>, NUMBER_OF_VALUES> line2;

    size_t counter = 0;
    for (auto& pair : line1) {
        pair.first = counter;
        if (counter++ < JUMP_AT) {
            pair.second = 0;
        } else {
            pair.second = 1;
        }
    }

    counter = 0;
    for (auto& pair : line2) {
        pair.first = counter++;
        pair.second = 0;
    }

    float output;
    float input;
    float setPoint;

    dev::PIDController pid(input, output, setPoint, 0, 0, 0.05, dev::PIDController::ControlDirection::DIRECT);
    pid.setSampleTime(std::chrono::milliseconds(1));
    pid.setMode(dev::PIDController::ControlMode::AUTOMATIC);
    pid.setOutputLimits(-1000, 1000);

    for (int i = 0; i < NUMBER_OF_VALUES; i++) {
        input = line1[i].second;
        setPoint = 1;
        g_Ticks += std::chrono::milliseconds(1).count();
        pid.compute();
        line2[i].second = output;
    }

    counter = 0;
    for (auto pair : line2) {
        if (counter == JUMP_AT) {
            CHECK(pair.second <= -49.49 && pair.second >= -49.51);
        } else if (counter == 0) {
            CHECK(pair.second >= 50.49 && pair.second <= 50.51);
        } else {
            CHECK(pair.second <= 0.52 && pair.second >= 0.48);
        }
        counter++;
    }

    plotLines(line1, line2);

    TestCaseEnd();
}
int main(int argc, const char* argv[])
{
    UnitTestMainBegin();
    RunTest(true, ut_TestPIDNoChange);
    RunTest(true, ut_TestPID_P_Change);
    RunTest(true, ut_TestPID_I_Change);
    RunTest(true, ut_TestPID_D_Change);
    UnitTestMainEnd();
}
