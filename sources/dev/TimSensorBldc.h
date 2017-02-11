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

#ifndef SOURCES_PMD_SENSOR_BLDC_H_
#define SOURCES_PMD_SENSOR_BLDC_H_

#include <cstdint>
#include <array>
#include "dev_Factory.h"
#include "TimHalfBridge.h"
#include "TimHallDecoder.h"
#include "TimHallMeter.h"
#include "PhaseCurrentSensor.h"

namespace dev
{
struct SensorBLDC {
    enum Description {
        BLDC,
        __ENUM__SIZE
    };

    enum class Direction
    {
        FORWARD,
        BACKWARD
    };

    SensorBLDC() = delete;
    SensorBLDC(const SensorBLDC&) = delete;
    SensorBLDC(SensorBLDC &&) = default;
    SensorBLDC& operator=(const SensorBLDC&) = delete;
    SensorBLDC& operator=(SensorBLDC &&) = delete;

    float getActualRPS(void) const;
    float getActualOmega(void) const;
    Direction getActualDirection(void) const;
    int32_t getActualPulsWidthPerMill(void) const;
    float getActualPhaseCurrent(void) const;
    float getActualTorqueInNewtonMeter(void) const;

    uint32_t getNumberOfPolePairs(void) const;

    Direction getSetDirection(void) const;
    void setDirection(const Direction) const;

    void calibrate(void) const;
    void setPulsWidthInMill(int32_t) const;
    void start(void) const;
    void stop(void) const;
    void checkMotor(void) const;

    const enum Description mDescription;

    const float mMotorConstant = 0.0;
    const float mMotorCoilResistance = 0.0;
    const float mMotorGeneratorConstant = 0.0;

    const hal::PhaseCurrentSensor& mPhaseCurrentSensor;

private:
    constexpr SensorBLDC(const enum Description&        desc,
                         const float                    motorConstant,
                         const float                    motorCoilResistance,
                         const float                    motorGeneratorConstant,
                         const hal::PhaseCurrentSensor& currentSensor,
                         const hal::HalfBridge&         hBridge,
                         const hal::HallDecoder&        hallDecoder,
                         const hal::HallMeter&          hallMeter1,
                         const hal::HallMeter&          hallMeter2) :
        mDescription(desc),
        mMotorConstant(motorConstant),
        mMotorCoilResistance(motorCoilResistance),
        mMotorGeneratorConstant(motorGeneratorConstant),
        mPhaseCurrentSensor(currentSensor),
        mHBridge(hBridge),
        mHallDecoder(hallDecoder),
        mHallMeter1(hallMeter1),
        mHallMeter2(hallMeter2)
    {}

    const hal::HalfBridge& mHBridge;
    const hal::HallDecoder& mHallDecoder;
    const hal::HallMeter& mHallMeter1;
    const hal::HallMeter& mHallMeter2;

    mutable Direction mSetDirection = Direction::FORWARD;
    mutable Direction mUpdateSetDirection = Direction::FORWARD;
    mutable Direction mCurrentDirection = Direction::FORWARD;
    mutable size_t mLastHallPosition = 0;
    mutable bool mManualCommutationActive = true;

    Direction getCurrentDirection(const size_t lastHallPosition, const size_t currentHallPosition) const;
    void computeDirection(void) const;
    void prepareCommutation(const size_t hallPosition) const;
    void manualCommutation(const size_t hallPosition) const;
    void commutate(const size_t hallPosition) const;
    void disableManualCommutation(const size_t hallPosition) const;
    void enableManualCommutation(void) const;
    void modifyPulsWidthPeriode(void) const;

    size_t getNextHallPosition(const size_t position) const;
    size_t getPreviousHallPosition(const size_t position) const;

    friend class Factory<SensorBLDC>;
};

template<>
class Factory<SensorBLDC>
{
    static constexpr std::array<const SensorBLDC, SensorBLDC::__ENUM__SIZE> Container =
    { {
          SensorBLDC(
                     SensorBLDC::BLDC,
                     0.065,
                     0.33,
                     144,
                     hal::Factory<hal::PhaseCurrentSensor>::get<hal::PhaseCurrentSensor::I_TOTAL_FB>(),
                     hal::Factory<hal::HalfBridge>::get<hal::HalfBridge::BLDC_PWM>(),
                     hal::Factory<hal::HallDecoder>::get<hal::HallDecoder::BLDC_DECODER>(),
                     hal::Factory<hal::HallMeter>::get<hal::HallMeter::BLDC_METER_32BIT>(),
                     hal::Factory<hal::HallMeter>::get<hal::HallMeter::BLDC_METER>())
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
