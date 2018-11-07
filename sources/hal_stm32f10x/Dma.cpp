// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include <cstring>
#include "Dma.h"
#include "trace.h"
#include "misc.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Dma;
using hal::Factory;

void Dma::DMA_IRQHandlerSemaphore(const Dma& dma, const Dma::SemaphoreArray& array)
{
    if (array[dma.mDescription] != nullptr) {
        array[dma.mDescription]->giveFromISR();
    }
}

void Dma::DMA_IRQHandlerCallback(const Dma& dma, const Dma::CallbackArray& array)
{
    if (array[dma.mDescription] != nullptr) {
        array[dma.mDescription]();
    }
}

void Dma::DMA_IRQHandler(const Dma& peripherie, const uint32_t TCFlag, const uint32_t HTFlag, const uint32_t TEFlag)
{
    if (DMA_GetITStatus(TCFlag)) {
        Dma::DMA_TCIRQHandler(peripherie);
        DMA_ClearITPendingBit(TCFlag);
    } else if (DMA_GetITStatus(HTFlag)) {
        DMA_ClearITPendingBit(HTFlag);
        Dma::DMA_HTIRQHandler(peripherie);
    } else if (DMA_GetITStatus(TEFlag)) {
        DMA_ClearITPendingBit(TEFlag);
        Dma::DMA_TEIRQHandler(peripherie);
    }
}

void Dma::DMA_TCIRQHandler(const Dma& peripherie)
{
    DMA_IRQHandlerSemaphore(peripherie, Dma::TCInterruptSemaphores);
    DMA_IRQHandlerCallback(peripherie, Dma::TCInterruptCallbacks);
}
void Dma::DMA_HTIRQHandler(const Dma& peripherie)
{
    DMA_IRQHandlerSemaphore(peripherie, Dma::HTInterruptSemaphores);
    DMA_IRQHandlerCallback(peripherie, Dma::HTInterruptCallbacks);
}
void Dma::DMA_TEIRQHandler(const Dma& peripherie)
{
    DMA_IRQHandlerSemaphore(peripherie, Dma::TEInterruptSemaphores);
    DMA_IRQHandlerCallback(peripherie, Dma::TEInterruptCallbacks);
}

#if DMA1_CHANNEL1_INTERRUPT_ENABLED
void DMA1_Channel1_IRQHandler(void)
{
    const uint32_t base = DMA1_Channel1_BASE;
    const uint32_t TCFlag = DMA1_IT_TC1;
    const uint32_t HTFlag = DMA1_IT_HT1;
    const uint32_t TEFlag = DMA1_IT_TE1;
    constexpr const auto& dma = Factory<Dma>::getByPeripherie<base>();
    Dma::DMA_IRQHandler(dma, TCFlag, HTFlag, TEFlag);
}
#endif

#if DMA1_CHANNEL2_INTERRUPT_ENABLED
void DMA1_Channel2_IRQHandler(void)
{
    const uint32_t base = DMA1_Channel2_BASE;
    const uint32_t TCFlag = DMA1_IT_TC2;
    const uint32_t HTFlag = DMA1_IT_HT2;
    const uint32_t TEFlag = DMA1_IT_TE2;
    constexpr const auto& dma = Factory<Dma>::getByPeripherie<base>();
    Dma::DMA_IRQHandler(dma, TCFlag, HTFlag, TEFlag);
}
#endif

#if DMA1_CHANNEL3_INTERRUPT_ENABLED
void DMA1_Channel3_IRQHandler(void)
{
    const uint32_t base = DMA1_Channel3_BASE;
    const uint32_t TCFlag = DMA1_IT_TC3;
    const uint32_t HTFlag = DMA1_IT_HT3;
    const uint32_t TEFlag = DMA1_IT_TE3;
    constexpr const auto& dma = Factory<Dma>::getByPeripherie<base>();
    Dma::DMA_IRQHandler(dma, TCFlag, HTFlag, TEFlag);
}
#endif

#if DMA1_CHANNEL4_INTERRUPT_ENABLED
void DMA1_Channel4_IRQHandler(void)
{
    const uint32_t base = DMA1_Channel4_BASE;
    const uint32_t TCFlag = DMA1_IT_TC4;
    const uint32_t HTFlag = DMA1_IT_HT4;
    const uint32_t TEFlag = DMA1_IT_TE4;
    constexpr const auto& dma = Factory<Dma>::getByPeripherie<base>();
    Dma::DMA_IRQHandler(dma, TCFlag, HTFlag, TEFlag);
}
#endif

#if DMA1_CHANNEL5_INTERRUPT_ENABLED
void DMA1_Channel5_IRQHandler(void)
{
    const uint32_t base = DMA1_Channel5_BASE;
    const uint32_t TCFlag = DMA1_IT_TC5;
    const uint32_t HTFlag = DMA1_IT_HT5;
    const uint32_t TEFlag = DMA1_IT_TE5;
    constexpr const auto& dma = Factory<Dma>::getByPeripherie<base>();
    Dma::DMA_IRQHandler(dma, TCFlag, HTFlag, TEFlag);
}
#endif

#if DMA1_CHANNEL6_INTERRUPT_ENABLED
void DMA1_Channel6_IRQHandler(void)
{
    const uint32_t base = DMA1_Channel6_BASE;
    const uint32_t TCFlag = DMA1_IT_TC6;
    const uint32_t HTFlag = DMA1_IT_HT6;
    const uint32_t TEFlag = DMA1_IT_TE6;
    constexpr const auto& dma = Factory<Dma>::getByPeripherie<base>();
    Dma::DMA_IRQHandler(dma, TCFlag, HTFlag, TEFlag);
}
#endif

#if DMA1_CHANNEL7_INTERRUPT_ENABLED
void DMA1_Channel7_IRQHandler(void)
{
    const uint32_t base = DMA1_Channel7_BASE;
    const uint32_t TCFlag = DMA1_IT_TC7;
    const uint32_t HTFlag = DMA1_IT_HT7;
    const uint32_t TEFlag = DMA1_IT_TE7;
    constexpr const auto& dma = Factory<Dma>::getByPeripherie<base>();
    Dma::DMA_IRQHandler(dma, TCFlag, HTFlag, TEFlag);
}
#endif

#if DMA2_CHANNEL1_INTERRUPT_ENABLED
void DMA2_Channel1_IRQHandler(void)
{
    const uint32_t base = DMA2_Channel1_BASE;
    const uint32_t TCFlag = DMA2_IT_TC1;
    const uint32_t HTFlag = DMA2_IT_HT1;
    const uint32_t TEFlag = DMA2_IT_TE1;
    constexpr const auto& dma = Factory<Dma>::getByPeripherie<base>();
    Dma::DMA_IRQHandler(dma, TCFlag, HTFlag, TEFlag);
}
#endif

#if DMA2_CHANNEL2_INTERRUPT_ENABLED
void DMA2_Channel2_IRQHandler(void)
{
    const uint32_t base = DMA2_Channel2_BASE;
    const uint32_t TCFlag = DMA2_IT_TC2;
    const uint32_t HTFlag = DMA2_IT_HT2;
    const uint32_t TEFlag = DMA2_IT_TE2;
    constexpr const auto& dma = Factory<Dma>::getByPeripherie<base>();
    Dma::DMA_IRQHandler(dma, TCFlag, HTFlag, TEFlag);
}
#endif

#if DMA2_CHANNEL3_INTERRUPT_ENABLED
void DMA2_Channel3_IRQHandler(void)
{
    const uint32_t base = DMA2_Channel3_BASE;
    const uint32_t TCFlag = DMA2_IT_TC3;
    const uint32_t HTFlag = DMA2_IT_HT3;
    const uint32_t TEFlag = DMA2_IT_TE3;
    constexpr const auto& dma = Factory<Dma>::getByPeripherie<base>();
    Dma::DMA_IRQHandler(dma, TCFlag, HTFlag, TEFlag);
}
#endif

#if DMA2_CHANNEL4_INTERRUPT_ENABLED
void DMA2_Channel4_IRQHandler(void)
{
    const uint32_t base = DMA2_Channel4_BASE;
    const uint32_t TCFlag = DMA2_IT_TC4;
    const uint32_t HTFlag = DMA2_IT_HT4;
    const uint32_t TEFlag = DMA2_IT_TE4;
    constexpr const auto& dma = Factory<Dma>::getByPeripherie<base>();
    Dma::DMA_IRQHandler(dma, TCFlag, HTFlag, TEFlag);
}
#endif

#if DMA2_CHANNEL5_INTERRUPT_ENABLED
void DMA2_Channel5_IRQHandler(void)
{
    const uint32_t base = DMA2_Channel5_BASE;
    const uint32_t TCFlag = DMA2_IT_TC5;
    const uint32_t HTFlag = DMA2_IT_HT5;
    const uint32_t TEFlag = DMA2_IT_TE5;
    constexpr const auto& dma = Factory<Dma>::getByPeripherie<base>();
    Dma::DMA_IRQHandler(dma, TCFlag, HTFlag, TEFlag);
}
#endif

void Dma::initialize(void) const
{
    DMA_Init(reinterpret_cast<DMA_Channel_TypeDef*>(mPeripherie), &mConfiguration);
    if (mDmaInterrupt) {
        DMA_ITConfig(reinterpret_cast<DMA_Channel_TypeDef*>(mPeripherie), mDmaInterrupt, ENABLE);
        NVIC_SetPriority(mDmaIRQn, 0xa);
        NVIC_EnableIRQ(mDmaIRQn);
    }
}

void Dma::setupTransfer(uint8_t const* const data, const size_t length, const bool repeat) const
{
    DMA_InitTypeDef initStruct = mConfiguration;
    initStruct.DMA_BufferSize = length;
    initStruct.DMA_MemoryBaseAddr = reinterpret_cast<uint32_t>(data);
    if (repeat) {
        initStruct.DMA_Mode = DMA_Mode_Circular;
    } else {
        initStruct.DMA_Mode = DMA_Mode_Normal;
    }

    disable();
    DMA_Init(reinterpret_cast<DMA_Channel_TypeDef*>(mPeripherie), &initStruct);
    if (mDmaInterrupt) {
        DMA_ITConfig(reinterpret_cast<DMA_Channel_TypeDef*>(mPeripherie), mDmaInterrupt, ENABLE);
    }
}

void Dma::memcpy(void const* const dest, void const* const src, const size_t length) const
{
    if ((dest == nullptr) || (src == nullptr) || (length == 0)) {
        return;
    }

    disable();

    const DMA_InitTypeDef initStruct {
        reinterpret_cast<uint32_t>(src),
        reinterpret_cast<uint32_t>(dest),
        DMA_DIR_PeripheralSRC,
        static_cast<uint16_t>(length),
        DMA_PeripheralInc_Enable,
        DMA_MemoryInc_Enable,
        DMA_PeripheralDataSize_Byte,
        DMA_MemoryDataSize_Byte,
        DMA_Mode_Normal,
        DMA_Priority_Low,
        DMA_M2M_Enable
    };

    DMA_Init(reinterpret_cast<DMA_Channel_TypeDef*>(mPeripherie), &initStruct);
    DMA_Cmd(reinterpret_cast<DMA_Channel_TypeDef*>(mPeripherie), ENABLE);
}

void Dma::setupSendSingleCharMultipleTimes(uint8_t const* const data, const size_t length) const
{
    disable();

    DMA_InitTypeDef initStruct = mConfiguration;
    initStruct.DMA_BufferSize = length;
    initStruct.DMA_MemoryBaseAddr = (uint32_t)data;
    initStruct.DMA_MemoryInc = DISABLE;

    DMA_Init(reinterpret_cast<DMA_Channel_TypeDef*>(mPeripherie), &initStruct);
    if (mDmaInterrupt) {
        DMA_ITConfig(reinterpret_cast<DMA_Channel_TypeDef*>(mPeripherie), mDmaInterrupt, ENABLE);
    }
}

void Dma::enable(void) const
{
    DMA_Cmd(reinterpret_cast<DMA_Channel_TypeDef*>(mPeripherie), ENABLE);
}

void Dma::disable(void) const
{
    DMA_Cmd(reinterpret_cast<DMA_Channel_TypeDef*>(mPeripherie), DISABLE);
}

bool Dma::registerInterruptSemaphore(os::Semaphore* const semaphore, const Dma::InterruptSource source) const
{
    switch (source) {
    case Dma::TC:
        if (mDmaInterrupt & DMA_IT_TC) {
            Dma::TCInterruptSemaphores[mDescription] = semaphore;
            return true;
        } else {
            Trace(ZONE_ERROR, "Invalid DMA configuration\r\n");
            return false;
        }

    case Dma::HT:
        if (mDmaInterrupt & DMA_IT_HT) {
            Dma::HTInterruptSemaphores[mDescription] = semaphore;
            return true;
        } else {
            Trace(ZONE_ERROR, "Invalid DMA configuration\r\n");
            return false;
        }

    case Dma::TE:
        if (mDmaInterrupt & DMA_IT_TE) {
            Dma::TEInterruptSemaphores[mDescription] = semaphore;
            return true;
        } else {
            Trace(ZONE_ERROR, "Invalid DMA configuration\r\n");
            return false;
        }
    }

    return false;
}

void Dma::unregisterInterruptSemaphore(const InterruptSource source) const
{
    switch (source) {
    case Dma::TC:
        Dma::TCInterruptSemaphores[mDescription] = nullptr;
        return;

    case Dma::HT:
        Dma::HTInterruptSemaphores[mDescription] = nullptr;
        return;

    case Dma::TE:
        Dma::TEInterruptSemaphores[mDescription] = nullptr;
        return;
    }
}

bool Dma::registerInterruptCallback(std::function<void(void)> function, const Dma::InterruptSource source) const
{
    switch (source) {
    case Dma::TC:
        if (mDmaInterrupt & DMA_IT_TC) {
            Dma::TCInterruptCallbacks[mDescription] = function;
            return true;
        } else {
            Trace(ZONE_ERROR, "Invalid DMA configuration\r\n");
            return false;
        }

    case Dma::HT:
        if (mDmaInterrupt & DMA_IT_HT) {
            Dma::HTInterruptCallbacks[mDescription] = function;
            return true;
        } else {
            Trace(ZONE_ERROR, "Invalid DMA configuration\r\n");
            return false;
        }

    case Dma::TE:
        if (mDmaInterrupt & DMA_IT_TE) {
            Dma::TEInterruptCallbacks[mDescription] = function;
            return true;
        } else {
            Trace(ZONE_ERROR, "Invalid DMA configuration\r\n");
            return false;
        }
    }

    return false;
}

void Dma::unregisterInterruptCallback(const InterruptSource source) const
{
    switch (source) {
    case Dma::TC:
        Dma::TCInterruptCallbacks[mDescription] = nullptr;
        return;

    case Dma::HT:
        Dma::HTInterruptCallbacks[mDescription] = nullptr;
        return;

    case Dma::TE:
        Dma::TEInterruptCallbacks[mDescription] = nullptr;
        return;
    }
}

uint16_t Dma::getCurrentDataCounter(void) const
{
    return DMA_GetCurrDataCounter(reinterpret_cast<DMA_Channel_TypeDef*>(mPeripherie));
}

void Dma::setCurrentDataCounter(uint16_t value) const
{
    DMA_SetCurrDataCounter(reinterpret_cast<DMA_Channel_TypeDef*>(mPeripherie), value);
}

Dma::SemaphoreArray Dma::TCInterruptSemaphores;
Dma::SemaphoreArray Dma::HTInterruptSemaphores;
Dma::SemaphoreArray Dma::TEInterruptSemaphores;
Dma::CallbackArray Dma::TCInterruptCallbacks;
Dma::CallbackArray Dma::HTInterruptCallbacks;
Dma::CallbackArray Dma::TEInterruptCallbacks;

constexpr const std::array<const Dma, Dma::Description::__ENUM__SIZE + 1> Factory<Dma>::Container;
constexpr const std::array<const uint32_t, 2> Factory<Dma>::Clocks;
