// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "Tim.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Factory;
using hal::Tim;

void Tim::setCounterValue(const uint32_t value) const
{
    TIM_SetCounter(getBasePointer(), value);
}

void Tim::setAutoReloadValue(const uint32_t value) const
{
    TIM_SetAutoreload(getBasePointer(), value);
}

uint32_t Tim::getCounterValue(void) const
{
    return TIM_GetCounter(getBasePointer());
}

uint32_t Tim::getTimerFrequency(void) const
{
#ifdef STM32F303xC
    if ((this->mPeripherie == TIM2_BASE) || (this->mPeripherie == TIM3_BASE)) {
        mClockFrequency = 72000000;
    }
#endif

    if (mConfiguration.TIM_ClockDivision == TIM_CKD_DIV1) {
        return mClockFrequency / (mConfiguration.TIM_Prescaler + 1);
    } else if (mConfiguration.TIM_ClockDivision == TIM_CKD_DIV2) {
        return (mClockFrequency / 2) / (mConfiguration.TIM_Prescaler + 1);
    } else {
        return (mClockFrequency / 4) / (mConfiguration.TIM_Prescaler + 1);
    }
}

uint32_t Tim::getPeriode(void) const
{
    return getBasePointer()->ARR;
}

void Tim::setPeriode(const uint32_t period) const
{
    setAutoReloadValue(period);
}

void Tim::enable(void) const
{
    TIM_Cmd(getBasePointer(), ENABLE);
}

void Tim::disable(void) const
{
    TIM_Cmd(getBasePointer(), DISABLE);
}

void Tim::selectOutputTrigger(uint16_t TRGO_Source) const
{
    TIM_SelectOutputTrigger(getBasePointer(), TRGO_Source);
}

ITStatus Tim::getInterruptStatus(const uint16_t interruptFlag) const
{
    return TIM_GetITStatus(getBasePointer(), interruptFlag);
}

void Tim::clearPendingInterruptFlag(const uint16_t interruptFlag) const
{
    TIM_ClearITPendingBit(getBasePointer(), interruptFlag);
}

void Tim::initialize(void) const
{
    TIM_DeInit(getBasePointer());
    TIM_TimeBaseInit(getBasePointer(), &mConfiguration);
}

TIM_TypeDef* Tim::getBasePointer(void) const
{
    return reinterpret_cast<TIM_TypeDef*>(mPeripherie);
}

constexpr const std::array<const Tim, Tim::Description::__ENUM__SIZE + 1> Factory<Tim>::Container;
constexpr const std::array<const uint32_t, Tim::Description::__ENUM__SIZE> Factory<Tim>::Clocks;
