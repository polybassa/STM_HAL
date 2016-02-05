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
#include "Adc.h"
#include "dev_Factory.h"
#include "TemperatureSensor.h"

#define DoPlot true

//--------------------------BUFFERS--------------------------
float gCurrentVoltage;

//--------------------------MOCKING--------------------------
float hal::Adc::Channel::getVoltage(void) const
{
    return gCurrentVoltage;
}

constexpr const std::array<const hal:: Adc::Channel,
                           hal::Adc::Channel::__ENUM__SIZE> hal::Factory<hal::Adc>::ChannelContainer;

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

    auto& testee = dev::Factory<dev::TemperatureSensor>::get<dev::TemperatureSensor::Description::MOTOR>();

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
