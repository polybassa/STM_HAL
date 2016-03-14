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
