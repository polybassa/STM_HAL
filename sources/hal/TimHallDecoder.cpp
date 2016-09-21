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
#include <cmath>
#include "Gpio.h"
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
    SEGGER_SYSVIEW_RecordEnterISR();
    SEGGER_SYSVIEW_Print("Hall");
    constexpr auto& hallDecoder = Factory<HallDecoder>::get<HallDecoder::BLDC_DECODER>();
    hallDecoder.interruptHandler();
    SEGGER_SYSVIEW_RecordExitISR();
}

void HallDecoder::interruptHandler(void) const
{
    if (TIM_GetITStatus(mTim.getBasePointer(), TIM_IT_CC1)) {
        TIM_ClearITPendingBit(mTim.getBasePointer(), TIM_IT_CC1);

        const uint32_t eventTimestamp = TIM_GetCapture1(mTim.getBasePointer());
        saveTimestamp(eventTimestamp);

        HallEventCallbacks[mDescription]();
    } else if (TIM_GetITStatus(mTim.getBasePointer(), TIM_IT_CC2)) {
        TIM_ClearITPendingBit(mTim.getBasePointer(), TIM_IT_CC2);

        HallDecoder::CommutationCallbacks[mDescription]();
    } else if (TIM_GetITStatus(mTim.getBasePointer(), TIM_IT_CC3)) {
        TIM_ClearITPendingBit(mTim.getBasePointer(), TIM_IT_CC3);
        // no hall interrupt, overflow occurred because of stall motor
        reset();
    } else {
        // this should not happen
    }
}

void HallDecoder::saveTimestamp(const uint32_t timestamp) const
{
    mTimestamps[mTimestampPosition] = timestamp;
    mTimestampPosition = (mTimestampPosition + 1) % NUMBER_OF_TIMESTAMPS;
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

    uint32_t sumTicksBetweenHallSignals =
        std::accumulate(mTimestamps.begin(), mTimestamps.end(), 0);

    const uint32_t avgTicksBetweenHallSignals = sumTicksBetweenHallSignals / NUMBER_OF_TIMESTAMPS; // is wrong

    const float timerFrequency = mTim.getTimerFrequency();
    const float hallSignalFrequency = timerFrequency / avgTicksBetweenHallSignals;
    const float electricalRotationFrequency = hallSignalFrequency / HALL_EVENTS_PER_ROTATION;
    const float motorRotationFrequency = electricalRotationFrequency / POLE_PAIRS;
    return motorRotationFrequency;
}

float HallDecoder::getCurrentOmega(void) const
{
    return getCurrentRPS() * M_PI * 2;
}

void HallDecoder::reset(void) const
{
    mTimestamps.fill(std::numeric_limits<uint32_t>::max());
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
    reset();

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
    TIM_SelectOutputTrigger(mTim.getBasePointer(), (uint16_t)TIM_TRGOSource_OC2Ref);

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
