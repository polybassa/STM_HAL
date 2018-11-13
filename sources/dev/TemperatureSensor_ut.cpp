// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "unittest.h"
#include "Adc.h"
#include "dev_Factory.h"
#include "TemperatureSensor.h"

#define DoPlot false

//--------------------------BUFFERS--------------------------
float gCurrentVoltage;

//--------------------------MOCKING--------------------------
uint32_t hal::Adc::Channel::getValue(void) const
{
    return gCurrentVoltage;
}

float hal::Adc::Channel::getVoltage(void) const
{
    return gCurrentVoltage;
}

constexpr const std::array<const hal::Adc::Channel,
                           hal::Adc::Channel::__ENUM__SIZE> hal::Factory<hal::Adc::Channel>::Container;
constexpr const std::array<const hal::Adc,
                           hal::Adc::__ENUM__SIZE> hal::Factory<hal::Adc>::Container;

//-------------------------HELPERFUNCTIONS-------------------
template<size_t n>
void plotLines(std::array<std::pair<float, float>, n> line1, std::array<std::pair<float, float>, n> line2)
{
#if DoPlot
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

int ut_TemperatureNTC_Read(void)
{
    TestCaseBegin();

    auto& testee = dev::Factory<dev::TemperatureSensor>::get<interface ::TemperatureSensor::Description::MOTOR>();

    std::array<std::pair<float, float>, 40> mVoltages;
    std::array<std::pair<float, float>, 40> mResults;

    const float maxVoltage = 3080;
    const float minVoltage = 179;
    const float thousand = 1000;
    const float factor = (maxVoltage - minVoltage) / (mVoltages.size() - 1);

    uint32_t counter = 0;
    for (auto& voltage : mVoltages) {
        voltage.first = counter;
        voltage.second = (counter++ *factor + minVoltage) / thousand;
    }

    counter = 0;
    for (auto& result : mResults) {
        gCurrentVoltage = mVoltages[counter].second;
        result.first = counter++;
        result.second = testee.getTemperature();
    }

    plotLines(mVoltages, mResults);

    TestCaseEnd();
}

int main(int argc, const char* argv[])
{
    UnitTestMainBegin();
    RunTest(true, ut_TemperatureNTC_Read);
    UnitTestMainEnd();
}
