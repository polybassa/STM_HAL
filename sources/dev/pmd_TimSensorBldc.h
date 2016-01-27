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

#ifndef SOURCES_PMD_SENSOR_BLDC_H_
#define SOURCES_PMD_SENSOR_BLDC_H_

#include <cstdint>
#include <array>
#include "pmd_dev_Factory.h"
#include "pmd_TimHalfBridge.h"
#include "pmd_TimHallDecoder.h"

namespace dev
{
struct SensorBLDC {
    enum Description {
        BLDC,
        __ENUM__SIZE
    };

    enum class Direction {
        FORWARD,
        BACKWARD
    };

    SensorBLDC() = delete;
    SensorBLDC(const SensorBLDC&) = delete;
    SensorBLDC(SensorBLDC&&) = default;
    SensorBLDC& operator=(const SensorBLDC&) = delete;
    SensorBLDC& operator=(SensorBLDC&&) = delete;

    void incrementCommutationDelay(void) const;
    void decrementCommutationDelay(void) const;
    void setCommutationDelay(const uint32_t) const;
    uint32_t getCommutationDelay(void) const;
    float getCurrentRPS(void) const;
    float getCurrentOmega(void) const;
    Direction getDirection(void) const;
    int32_t getPulsWidthPerMill(void) const;
    void setPulsWidthInMill(int32_t) const;
    void setDirection(const Direction) const;
    void trigger(void) const;

    void checkMotor(void) const;

    void start(void) const;
    void stop(void) const;

private:
    constexpr SensorBLDC(const enum Description& desc,
                         const hal::HalfBridge&  hBridge,
                         const hal::HallDecoder& hallDecoder) : mDescription(desc), mHBridge(hBridge),
        mHallDecoder(hallDecoder) {}

    const enum Description mDescription;
    const hal::HalfBridge& mHBridge;
    const hal::HallDecoder& mHallDecoder;

    mutable Direction mDirection = Direction::FORWARD;
    mutable size_t mLastHallPosition = 0;

    bool checkHallEvent(void) const;
    void prepareCommutation(const size_t hallPosition) const;
    size_t getNextHallPosition(const size_t position) const;
    size_t getPreviousHallPosition(const size_t position) const;

    friend class Factory<SensorBLDC>;
};

template<>
class Factory<SensorBLDC> {
    static constexpr std::array<const SensorBLDC, SensorBLDC::__ENUM__SIZE> Container =
    { {
          SensorBLDC(
              SensorBLDC::BLDC,
              hal::Factory<hal::HalfBridge>::get<hal::HalfBridge::BLDC_PWM>(),
              hal::Factory<hal::HallDecoder>::get<hal::HallDecoder::BLDC_DECODER>()
              )
      } };

public:

    template<enum SensorBLDC::Description index>
    static constexpr const SensorBLDC& get(void)
    {
        static_assert(Container[index].mHallDecoder.mDescription != hal::HallDecoder::Description::__ENUM__SIZE,
                      "Invalid Tim Object");
        static_assert(Container[index].mHBridge.mDescription != hal::HalfBridge::Description::__ENUM__SIZE,
                      "Invalid Tim Object");
        static_assert(index != SensorBLDC::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_SENSOR_BLDC_H_ */
