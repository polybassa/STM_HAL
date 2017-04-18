/* Copyright (C) 2015  Matthias Birnthaler
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

#ifndef SOURCES_PMD_STRAINGAUGESENSOR_H_
#define SOURCES_PMD_STRAINGAUGESENSOR_H_

#include <cstdint>
#include <limits>
#include <array>
#include "Adc.h"
#include "AdcChannel.h"
#include "dev_Factory.h"

#ifdef UNITTEST
extern int ut_StraingaugeSensorMeanTest(void);
extern int ut_StraingaugeSensorMeanTest1(void);
#endif

namespace dev
{
struct StraingaugeSensor {
    enum Description {
        STRAINGAUGESENSOR,
        __ENUM__SIZE
    };

    StraingaugeSensor() = delete;
    StraingaugeSensor(const StraingaugeSensor&) = delete;
    StraingaugeSensor(StraingaugeSensor &&) = default;
    StraingaugeSensor& operator=(const StraingaugeSensor&) = delete;
    StraingaugeSensor& operator=(StraingaugeSensor &&) = delete;

    void resetStraingaugeSensor(void) const;
    float getDirection(void) const;

private:

    constexpr StraingaugeSensor(const enum Description&  desc,
                                const hal::Adc::Channel& peripherie) :
        mDescription(desc), mPeripherie(peripherie){}

    const enum Description mDescription;
    const hal::Adc::Channel& mPeripherie;

    static constexpr size_t FILTERSIZE = 30;
    mutable std::array<float, FILTERSIZE> mFilterValues = {{}};
    mutable uint16_t mFirstFillCount = 1;
    mutable size_t mCurrentValueFilterIndex = 0;
    mutable float mLastFilterValue = 0.0;

    mutable float mGlobalMean = 0.0;
    mutable float mMean = 0.0;
    mutable float mMeanold = 0.0;
    mutable uint32_t mCurrentRange = 0;
    static constexpr size_t mMeanRange = 1000;

    static constexpr size_t mChangeFactor = 100;
    mutable float mFlow = 0.0;
    mutable float mTrail = 0.0;
    mutable float mChange = 0.0;

    float getRawStrain(void) const;
    void FilterStrain(void) const;
    void caculateMean(void) const;
    void caculateChange(void) const;

    friend class Factory<StraingaugeSensor>;

#ifdef UNITTEST
    friend int ::ut_StraingaugeSensorMeanTest(void);
    friend int ::ut_StraingaugeSensorMeanTest1(void);
#endif
};

template<>
class Factory<StraingaugeSensor>
{
    static constexpr const std::array<const StraingaugeSensor, StraingaugeSensor::__ENUM__SIZE> Container =
    { {
          StraingaugeSensor(StraingaugeSensor::STRAINGAUGESENSOR, hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::DMS>())
      } };

public:

    template<enum StraingaugeSensor::Description index>
    static constexpr const StraingaugeSensor& get(void)
    {
        static_assert(index != StraingaugeSensor::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }
};
}

#endif /* SOURCES_PMD_STRAINGAUGESENSOR_H_ */
