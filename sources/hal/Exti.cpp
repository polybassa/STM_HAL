/* Copyright (C) 2015  Nils Weiss, Markus Wildgruber
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

#include "Exti.h"
#include "trace.h"
#include <cstring>
#include <limits>
#include "stm32f30x_it.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Exti;
using hal::Factory;
using hal::Gpio;

#include "Exti_IRQ_config.cpp"

Exti::CallbackArray Exti::ExtiCallbacks;

void Exti::executeCallback(void) const
{
    if (ExtiCallbacks[mDescription]) {
        ExtiCallbacks[mDescription]();
    }
}

void Exti::handleInterrupt(void) const
{
    if (getStatus()) {
        clearInterruptBit();
        executeCallback();
    }
}

IRQn Exti::getIRQChannel(const EXTI_InitTypeDef& config) const
{
    switch (config.EXTI_Line) {
    case EXTI_Line0:
        return IRQn::EXTI0_IRQn;

    case EXTI_Line1:
        return IRQn::EXTI1_IRQn;

    case EXTI_Line2:
        return IRQn::EXTI2_TS_IRQn;

    case EXTI_Line3:
        return IRQn::EXTI3_IRQn;

    case EXTI_Line4:
        return IRQn::EXTI4_IRQn;

    case EXTI_Line5:
    case EXTI_Line6:
    case EXTI_Line7:
    case EXTI_Line8:
    case EXTI_Line9:
        return IRQn::EXTI9_5_IRQn;

    case EXTI_Line10:
    case EXTI_Line11:
    case EXTI_Line12:
    case EXTI_Line13:
    case EXTI_Line14:
    case EXTI_Line15:
        return IRQn::EXTI15_10_IRQn;
    }

    return IRQn::UsageFault_IRQn;
}

uint8_t Exti::getExtiPortSource(const uint32_t& peripherieBase) const
{
    switch (peripherieBase) {
    case GPIOA_BASE:
        return EXTI_PortSourceGPIOA;

    case GPIOB_BASE:
        return EXTI_PortSourceGPIOB;

    case GPIOC_BASE:
        return EXTI_PortSourceGPIOC;

    case GPIOD_BASE:
        return EXTI_PortSourceGPIOD;

    case GPIOE_BASE:
        return EXTI_PortSourceGPIOE;

    case GPIOF_BASE:
        return EXTI_PortSourceGPIOF;

    case GPIOG_BASE:
        return EXTI_PortSourceGPIOG;

    case GPIOH_BASE:
        return EXTI_PortSourceGPIOH;
    }

    return std::numeric_limits<uint8_t>::max();
}

void Exti::initialize(void) const
{
    SYSCFG_EXTILineConfig(getExtiPortSource(mGpio.mPeripherie), mGpio.mPinSource);
    EXTI_Init(&mConfiguration);

    const IRQn irqChannel = getIRQChannel(mConfiguration);

    NVIC_SetPriority(irqChannel, mPriority);
    NVIC_EnableIRQ(irqChannel);
}

void Exti::disable(void) const
{
    setEXTI_LineCmd(DISABLE);
}

void Exti::enable(void) const
{
    setEXTI_LineCmd(ENABLE);
}

void Exti::setEXTI_LineCmd(const FunctionalState cmd) const
{
    EXTI_InitTypeDef initStruct;
    std::memcpy(&initStruct, &mConfiguration, sizeof(EXTI_InitTypeDef));
    initStruct.EXTI_LineCmd = cmd;
    EXTI_Init(&initStruct);
}

bool Exti::getStatus(void) const
{
    return EXTI_GetITStatus(mConfiguration.EXTI_Line) == SET;
}

void Exti::clearInterruptBit(void) const
{
    EXTI_ClearITPendingBit(mConfiguration.EXTI_Line);
}

void Exti::registerInterruptCallback(std::function<void(void)> f) const
{
    ExtiCallbacks[mDescription] = f;
}
void Exti::unregisterInterruptCallback(void) const
{
    ExtiCallbacks[mDescription] = nullptr;
}

constexpr const std::array<const Exti, Exti::Description::__ENUM__SIZE> Factory<Exti>::Container;
