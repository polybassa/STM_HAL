// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "Exti.h"
#include "trace.h"
#include <cstring>
#include <limits>
#include "stm32f30x_it.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Exti;
using hal::Factory;
using hal::Gpio;

extern "C" {
void EXTI0_IRQHandler(void)
{
#if EXTI0_INTERRUPT_ENABLED
    constexpr const Exti& exti = hal::Factory<Exti>::getByExtiLine<EXTI_Line0>();
    exti.handleInterrupt();
#endif
}

void EXTI1_IRQHandler(void)
{
#if EXTI1_INTERRUPT_ENABLED
    constexpr const Exti& exti = hal::Factory<Exti>::getByExtiLine<EXTI_Line1>();
    exti.handleInterrupt();
#endif
}

void EXTI2_TSC_IRQHandler(void)
{
#if EXTI2_INTERRUPT_ENABLED
    constexpr const Exti& exti = hal::Factory<Exti>::getByExtiLine<EXTI_Line2>();
    exti.handleInterrupt();
#endif
}

void EXTI3_IRQHandler(void)
{
#if EXTI3_INTERRUPT_ENABLED
    constexpr const Exti& exti = hal::Factory<Exti>::getByExtiLine<EXTI_Line3>();
    exti.handleInterrupt();
#endif
}

void EXTI4_IRQHandler(void)
{
#if EXTI4_INTERRUPT_ENABLED
    constexpr const Exti& exti = hal::Factory<Exti>::getByExtiLine<EXTI_Line4>();
    exti.handleInterrupt();
#endif
}

void EXTI9_5_IRQHandler(void)
{
#if EXTI5_INTERRUPT_ENABLED
    constexpr const Exti& exti5 = hal::Factory<Exti>::getByExtiLine<EXTI_Line5>();
    exti5.handleInterrupt();
#endif
#if EXTI6_INTERRUPT_ENABLED
    constexpr const Exti& exti6 = hal::Factory<Exti>::getByExtiLine<EXTI_Line6>();
    exti6.handleInterrupt();
#endif
#if EXTI7_INTERRUPT_ENABLED
    constexpr const Exti& exti7 = hal::Factory<Exti>::getByExtiLine<EXTI_Line7>();
    exti7.handleInterrupt();
#endif
#if EXTI8_INTERRUPT_ENABLED
    constexpr const Exti& exti8 = hal::Factory<Exti>::getByExtiLine<EXTI_Line8>();
    exti8.handleInterrupt();
#endif
#if EXTI9_INTERRUPT_ENABLED
    constexpr const Exti& exti9 = hal::Factory<Exti>::getByExtiLine<EXTI_Line9>();
    exti9.handleInterrupt();
#endif
}

void EXTI15_10_IRQHandler(void)
{
#if EXTI10_INTERRUPT_ENABLED
    constexpr const Exti& exti10 = hal::Factory<Exti>::getByExtiLine<EXTI_Line10>();
    exti10.handleInterrupt();
#endif
#if EXTI11_INTERRUPT_ENABLED
    constexpr const Exti& exti11 = hal::Factory<Exti>::getByExtiLine<EXTI_Line11>();
    exti11.handleInterrupt();
#endif
#if EXTI12_INTERRUPT_ENABLED
    constexpr const Exti& exti12 = hal::Factory<Exti>::getByExtiLine<EXTI_Line12>();
    exti12.handleInterrupt();
#endif
#if EXTI13_INTERRUPT_ENABLED
    constexpr const Exti& exti13 = hal::Factory<Exti>::getByExtiLine<EXTI_Line13>();
    exti13.handleInterrupt();
#endif
#if EXTI14_INTERRUPT_ENABLED
    constexpr const Exti& exti14 = hal::Factory<Exti>::getByExtiLine<EXTI_Line14>();
    exti14.handleInterrupt();
#endif
#if EXTI15_INTERRUPT_ENABLED
    constexpr const Exti& exti15 = hal::Factory<Exti>::getByExtiLine<EXTI_Line15>();
    exti15.handleInterrupt();
#endif
}
}

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
