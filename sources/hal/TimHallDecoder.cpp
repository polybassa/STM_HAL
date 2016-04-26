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

#include <limits>
#include <algorithm>
#include <cstring>
#include <cmath>
#include "Gpio.h"
#include "Dma.h"
#include "TimHallDecoder.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Factory;
using hal::HallDecoder;
using hal::Tim;

#ifndef M_PI
static constexpr float M_PI = 3.14159265358979323846f;
#endif

extern "C" void TIM3_IRQHandler(void)
{
    constexpr auto& hallDecoder = Factory<HallDecoder>::get<HallDecoder::BLDC_DECODER>();
    hallDecoder.interruptHandler();
}

void HallDecoder::interruptHandler(void) const
{
    static bool hallEventDetected = false;

    if (TIM_GetITStatus(mTim.getBasePointer(), TIM_IT_CC1)) {
        TIM_ClearITPendingBit(mTim.getBasePointer(), TIM_IT_CC1);
        const uint32_t eventTimestamp = TIM_GetCapture1(mTim.getBasePointer());
        if (HallEventCallbacks[mDescription]()) {
            hallEventDetected = true;
            // calculate motor speed or else with CCR1 values
            saveTimestamp(eventTimestamp);
        } else {
            TIM_SetCounter(mTim.getBasePointer(), eventTimestamp);
        }
    } else if (TIM_GetITStatus(mTim.getBasePointer(), TIM_IT_CC2)) {
        TIM_ClearITPendingBit(mTim.getBasePointer(), TIM_IT_CC2);
        if (hallEventDetected) {
            HallDecoder::CommutationCallbacks[mDescription]();
            hallEventDetected = false;
        }
    } else if (TIM_GetITStatus(mTim.getBasePointer(), TIM_IT_CC3)) {
        TIM_ClearITPendingBit(mTim.getBasePointer(), TIM_IT_CC3);
        // no hall interrupt, overflow occurred because of stall motor
        saveTimestamp(std::numeric_limits<uint32_t>::max());
    } else {
        // this should not happen
    }
}

void HallDecoder::saveTimestamp(const uint32_t timestamp) const
{
    if (std::numeric_limits<uint32_t>::max() == timestamp) {
        mTimestamps.fill(std::numeric_limits<uint32_t>::max());
    }

    // move all values one step forward in array
    // to make space for next timestamp
    mTimestamps[NUMBER_OF_TIMESTAMPS - 1] = timestamp;
    uint32_t* dataPointer = mTimestamps.data();

    constexpr auto& dma = Factory<hal::Dma>::get<hal::Dma::MEMORY>();
    dma.memcpy(dataPointer,
               dataPointer + 1,
               (HallDecoder::NUMBER_OF_TIMESTAMPS - 1) * sizeof(uint32_t));
}

void HallDecoder::incrementCommutationDelay(void) const
{
    setCommutationDelay(getCommutationDelay() + 1);
}

void HallDecoder::decrementCommutationDelay(void) const
{
    setCommutationDelay(getCommutationDelay() - 1);
}

void HallDecoder::setCommutationDelay(const uint32_t value) const
{
    if ((value >= (std::numeric_limits<uint32_t>::min() + 1)) && (value < std::numeric_limits<uint32_t>::max())) {
        TIM_SetCompare2(mTim.getBasePointer(), value);
    }
}

uint32_t HallDecoder::getCommutationDelay(void) const
{
    return TIM_GetCapture2(mTim.getBasePointer());
}

float HallDecoder::getCurrentRPS(void) const
{
    static constexpr float HALL_EVENTS_PER_ROTATION = 6;
    // increment begin iterator to skip first value which is invalid
    auto begin = mTimestamps.begin();
    begin++;

    const uint32_t avgTicksBetweenHallSignals =
        std::accumulate(begin, mTimestamps.end(), 0) / (NUMBER_OF_TIMESTAMPS - 1);

    const float timerFrequency = SYSTEMCLOCK / (mTim.mConfiguration.TIM_Prescaler + 1);
    const float hallSignalFrequency = timerFrequency / avgTicksBetweenHallSignals;
    const float electricalRotationFrequency = hallSignalFrequency / HALL_EVENTS_PER_ROTATION;
    const float motorRotationFrequency = electricalRotationFrequency / POLE_PAIRS;
    return motorRotationFrequency;
}

float HallDecoder::getCurrentOmega(void) const
{
    return getCurrentRPS() * M_PI * 2;
}

uint32_t HallDecoder::getCurrentHallState(void) const
{
    if (mDescription == Description::BLDC_DECODER) {
        constexpr auto& hall1 = Factory<Gpio>::getAlternateFunctionGpio<Gpio::HALL1>();
        constexpr auto& hall2 = Factory<Gpio>::getAlternateFunctionGpio<Gpio::HALL2>();
        constexpr auto& hall3 = Factory<Gpio>::getAlternateFunctionGpio<Gpio::HALL3>();

        return (uint32_t)hall1 | ((uint32_t)hall2 << 1) | ((uint32_t)hall3 << 2);
    } else {
        return 0;
    }
}

void HallDecoder::registerCommutationCallback(std::function<void(void)> callback) const
{
    CommutationCallbacks[mDescription] = callback;
}

void HallDecoder::unregisterCommutationCallback(void) const
{
    registerCommutationCallback([] {});
}

void HallDecoder::registerHallEventCheckCallback(std::function<bool(void)> callback) const
{
    HallEventCallbacks[mDescription] = callback;
}

void HallDecoder::unregisterHallEventCheckCallback(void) const
{
    registerHallEventCheckCallback([] { return false;
                                   });
}

void HallDecoder::initialize(void) const
{
    mTimestamps.fill(std::numeric_limits<uint32_t>::max());

    /* T1F_ED will be connected to HallSensor inputs */
    TIM_SelectHallSensor(mTim.getBasePointer(), ENABLE);

    /* HallSensor event is delivered with signal TI1F_ED.
     * Rising and falling edge is used.*/
    TIM_SelectInputTrigger(mTim.getBasePointer(), TIM_TS_TI1F_ED);

    /* On every event, update is triggered and counter gets reset */
    TIM_SelectSlaveMode(mTim.getBasePointer(), TIM_SlaveMode_Reset);

    /* Channel 1 is in input capture mode. On every TCR edge, the counter
     * value is captured and stored into the CCR register. CCR1 interrupt is fired */
    TIM_ICInit(mTim.getBasePointer(), &mIc1Configuration);

    /* Cannel 2 is used for the commutation delay between hallsensor edge and
     * switching the FETs into the next step. */
    TIM_OC2Init(mTim.getBasePointer(), &mOc2Configuration);
    TIM_OC3Init(mTim.getBasePointer(), &mOc3Configuration);

    TIM_SelectMasterSlaveMode(mTim.getBasePointer(), TIM_MasterSlaveMode_Enable);

    /* Channel 2 output compare signal is connected to TRIGO */
    // Uncomment for automatic trigger of commutation
    TIM_SelectOutputTrigger(mTim.getBasePointer(), (uint16_t)TIM_TRGO2Source_OC2Ref);

    unregisterHallEventCheckCallback();
    unregisterCommutationCallback();

    TIM_ClearFlag(mTim.getBasePointer(), TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3);
    TIM_ITConfig(mTim.getBasePointer(), TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3, ENABLE);

    NVIC_SetPriority(IRQn_Type::TIM3_IRQn, 0x9);
    NVIC_EnableIRQ(IRQn_Type::TIM3_IRQn);
}

constexpr const std::array<const HallDecoder,
                           HallDecoder::Description::__ENUM__SIZE> Factory<HallDecoder>::Container;
std::array<std::function<void(void)>, HallDecoder::Description::__ENUM__SIZE> HallDecoder::CommutationCallbacks;
std::array<std::function<bool(void)>, HallDecoder::Description::__ENUM__SIZE> HallDecoder::HallEventCallbacks;
