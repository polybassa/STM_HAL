/* Copyright (C) 2018  Nils Weiss and Henning Mende
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

#include <cstring>
#include "Dma.h"
#include "trace.h"
#include "stm32f4xx_misc.h"

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

void Dma::DMA_IRQHandler(const Dma* const peripherie)
{
    DMA_Stream_TypeDef* pPeripheral = reinterpret_cast<DMA_Stream_TypeDef*>(peripherie->mPeripherie);
    const uint32_t TCFlag = getTCFlagForStream(peripherie->mPeripherie);
    const uint32_t HTFlag = getHTFlagForStream(peripherie->mPeripherie);
    const uint32_t TEFlag = getTEFlagForStream(peripherie->mPeripherie);

    if (DMA_GetFlagStatus(pPeripheral, TCFlag)) {
        Dma::DMA_TCIRQHandler(*peripherie);
    } else if (DMA_GetFlagStatus(pPeripheral, HTFlag)) {
        Dma::DMA_HTIRQHandler(*peripherie);
    } else if (DMA_GetFlagStatus(pPeripheral, TEFlag)) {
        Dma::DMA_TEIRQHandler(*peripherie);
    }
}

void Dma::ClearInterruptFlag(const Dma* const peripherie)
{
    DMA_Stream_TypeDef* pPeripheral = reinterpret_cast<DMA_Stream_TypeDef*>(peripherie->mPeripherie);
    const uint32_t TCFlag = getTCFlagForStream(peripherie->mPeripherie);
    const uint32_t HTFlag = getHTFlagForStream(peripherie->mPeripherie);
    const uint32_t TEFlag = getTEFlagForStream(peripherie->mPeripherie);

    if (DMA_GetFlagStatus(pPeripheral, TCFlag)) {
        DMA_ClearFlag(pPeripheral, TCFlag);
    } else if (DMA_GetFlagStatus(pPeripheral, HTFlag)) {
        DMA_ClearFlag(pPeripheral, HTFlag);
    } else if (DMA_GetFlagStatus(pPeripheral, TEFlag)) {
        DMA_ClearFlag(pPeripheral, TEFlag);
    }
}

bool Dma::GetInterruptFlagStatus(const Dma* const peripherie)
{
    DMA_Stream_TypeDef* pPeripheral = reinterpret_cast<DMA_Stream_TypeDef*>(peripherie->mPeripherie);
    const uint32_t TCFlag = getTCFlagForStream(peripherie->mPeripherie);
    const uint32_t HTFlag = getHTFlagForStream(peripherie->mPeripherie);
    const uint32_t TEFlag = getTEFlagForStream(peripherie->mPeripherie);

    return DMA_GetFlagStatus(pPeripheral, TCFlag) ||
           DMA_GetFlagStatus(pPeripheral, HTFlag) ||
           DMA_GetFlagStatus(pPeripheral, TEFlag);
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

void Dma::initialize(void) const
{
    DMA_Stream_TypeDef* pPeripheral = reinterpret_cast<DMA_Stream_TypeDef*>(mPeripherie);
    DMA_InitTypeDef tmpInitTypeDef = mConfiguration;
    DMA_Init(pPeripheral, &tmpInitTypeDef);
    if (mDmaInterrupt) {
        DMA_ITConfig(pPeripheral, mDmaInterrupt, ENABLE);

        mpNvic->registerClearInterruptProcedure([this](void) {
            Dma::ClearInterruptFlag(this);
        });
        mpNvic->registerGetInterruptStatusProcedure([this](void) -> bool {
            return Dma::GetInterruptFlagStatus(this);
        });
        mpNvic->registerInterruptCallback([this](void) -> void {
            Dma::DMA_IRQHandler(this);
        });
        mpNvic->enable();
    }
}

void Dma::setupTransfer(uint8_t const* const data, const size_t length, const bool repeat) const
{
    DMA_InitTypeDef initStruct = mConfiguration;
    initStruct.DMA_BufferSize = length;
    initStruct.DMA_Memory0BaseAddr = reinterpret_cast<uint32_t>(data);
    if (repeat) {
        initStruct.DMA_Mode = DMA_Mode_Circular;
    } else {
        initStruct.DMA_Mode = DMA_Mode_Normal;
    }

    disable();
    DMA_Init(reinterpret_cast<DMA_Stream_TypeDef*>(mPeripherie), &initStruct);
    if (mDmaInterrupt) {
        DMA_ITConfig(reinterpret_cast<DMA_Stream_TypeDef*>(mPeripherie), mDmaInterrupt, ENABLE);
    }
}

void Dma::memcpy(void const* const dest, void const* const src, const size_t length) const
{
    if ((dest == nullptr) || (src == nullptr) || (length == 0) || (mDescription != MEMORY)) {
        return;
    }

    disable();

    DMA_InitTypeDef initStruct {
        mConfiguration.DMA_Channel,
        reinterpret_cast<uint32_t>(src),
        reinterpret_cast<uint32_t>(dest),
        DMA_DIR_MemoryToMemory,
        static_cast<uint16_t>(length),
        DMA_PeripheralInc_Enable,
        DMA_MemoryInc_Enable,
        DMA_PeripheralDataSize_Byte,
        DMA_MemoryDataSize_Byte,
        DMA_Mode_Normal,
        DMA_Priority_Low,
        DMA_FIFOMode_Disable,
        DMA_FIFOThreshold_Full,
        DMA_MemoryBurst_Single,
        DMA_PeripheralBurst_Single
    };
    DMA_Stream_TypeDef* pPeripheral = reinterpret_cast<DMA_Stream_TypeDef*>(mPeripherie);
    DMA_Init(pPeripheral, &initStruct);
    DMA_Cmd(pPeripheral, ENABLE);
}

void Dma::setupSendSingleCharMultipleTimes(uint8_t const* const data, const size_t length) const
{
    disable();

    DMA_InitTypeDef initStruct = mConfiguration;
    initStruct.DMA_BufferSize = length;
    initStruct.DMA_Memory0BaseAddr = (uint32_t)data;
    initStruct.DMA_MemoryInc = DISABLE;

    DMA_Init(reinterpret_cast<DMA_Stream_TypeDef*>(mPeripherie), &initStruct);
    if (mDmaInterrupt) {
        DMA_ITConfig(reinterpret_cast<DMA_Stream_TypeDef*>(mPeripherie), mDmaInterrupt, ENABLE);
    }
}

void Dma::enable(void) const
{
    DMA_Cmd(reinterpret_cast<DMA_Stream_TypeDef*>(mPeripherie), ENABLE);
}

void Dma::disable(void) const
{
    DMA_Cmd(reinterpret_cast<DMA_Stream_TypeDef*>(mPeripherie), DISABLE);
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
    return DMA_GetCurrDataCounter(reinterpret_cast<DMA_Stream_TypeDef*>(mPeripherie));
}

void Dma::setCurrentDataCounter(uint16_t value) const
{
    DMA_SetCurrDataCounter(reinterpret_cast<DMA_Stream_TypeDef*>(mPeripherie), value);
}

uint32_t Dma::getITForStream(const uint32_t& DMAy_Streamx,
                             const uint32_t& DMA_IT)
{
    switch (DMA_IT) {
    case DMA_IT_TC:
        return getTCFlagForStream(DMAy_Streamx);

    case DMA_IT_HT:
        return getHTFlagForStream(DMAy_Streamx);

    case DMA_IT_TE:
        return getTEFlagForStream(DMAy_Streamx);

    case DMA_IT_DME:
        return getDMEFlagForStream(DMAy_Streamx);

    case DMA_IT_FE:
        return getFEFlagForStream(DMAy_Streamx);

    default:
        return 0;
    }
}

uint32_t Dma::getFEFlagForStream(const uint32_t& DMAy_Streamx)
{
    // get the bits indicating the stream and devide by the bit offset between the streams
    const uint32_t stream = (DMAy_Streamx & 0xFF) / 0x18;
    uint32_t dma_flag = 0;

    switch (stream) {
    case 0:
        dma_flag = DMA_FLAG_FEIF0;
        break;

    case 1:
        dma_flag = DMA_FLAG_FEIF1;
        break;

    case 2:
        dma_flag = DMA_FLAG_FEIF2;
        break;

    case 3:
        dma_flag = DMA_FLAG_FEIF3;
        break;

    case 4:
        dma_flag = DMA_FLAG_FEIF4;
        break;

    case 5:
        dma_flag = DMA_FLAG_FEIF5;
        break;

    case 6:
        dma_flag = DMA_FLAG_FEIF6;
        break;

    case 7:
        dma_flag = DMA_FLAG_FEIF7;
        break;

    default:
        // nothing to do in default case
        break;
    }

    return dma_flag;
}

uint32_t Dma::getDMEFlagForStream(const uint32_t& DMAy_Streamx)
{
    // get the bits indicating the stream and devide by the bit offset between the streams
    const uint32_t stream = (DMAy_Streamx & 0xFF) / 0x18;
    uint32_t dma_flag = 0;

    switch (stream) {
    case 0:
        dma_flag = DMA_FLAG_DMEIF0;
        break;

    case 1:
        dma_flag = DMA_FLAG_DMEIF1;
        break;

    case 2:
        dma_flag = DMA_FLAG_DMEIF2;
        break;

    case 3:
        dma_flag = DMA_FLAG_DMEIF3;
        break;

    case 4:
        dma_flag = DMA_FLAG_DMEIF4;
        break;

    case 5:
        dma_flag = DMA_FLAG_DMEIF5;
        break;

    case 6:
        dma_flag = DMA_FLAG_DMEIF6;
        break;

    case 7:
        dma_flag = DMA_FLAG_DMEIF7;
        break;

    default:
        // nothing to do here
        break;
    }

    return dma_flag;
}

uint32_t Dma::getTEFlagForStream(const uint32_t& DMAy_Streamx)
{
    // get the bits indicating the stream and devide by the bit offset between the streams
    const uint32_t stream = (DMAy_Streamx & 0xFF) / 0x18;
    uint32_t dma_flag = 0;

    switch (stream) {
    case 0:
        dma_flag = DMA_FLAG_TEIF0;
        break;

    case 1:
        dma_flag = DMA_FLAG_TEIF1;
        break;

    case 2:
        dma_flag = DMA_FLAG_TEIF2;
        break;

    case 3:
        dma_flag = DMA_FLAG_TEIF3;
        break;

    case 4:
        dma_flag = DMA_FLAG_TEIF4;
        break;

    case 5:
        dma_flag = DMA_FLAG_TEIF5;
        break;

    case 6:
        dma_flag = DMA_FLAG_TEIF6;
        break;

    case 7:
        dma_flag = DMA_FLAG_TEIF7;
        break;

    default:
        // nothing to do here
        break;
    }

    return dma_flag;
}

uint32_t Dma::getHTFlagForStream(const uint32_t& DMAy_Streamx)
{
    // get the bits indicating the stream and devide by the bit offset between the streams
    const uint32_t stream = (DMAy_Streamx & 0xFF) / 0x18;
    uint32_t dma_flag = 0;

    switch (stream) {
    case 0:
        dma_flag = DMA_FLAG_HTIF0;
        break;

    case 1:
        dma_flag = DMA_FLAG_HTIF1;
        break;

    case 2:
        dma_flag = DMA_FLAG_HTIF2;
        break;

    case 3:
        dma_flag = DMA_FLAG_HTIF3;
        break;

    case 4:
        dma_flag = DMA_FLAG_HTIF4;
        break;

    case 5:
        dma_flag = DMA_FLAG_HTIF5;
        break;

    case 6:
        dma_flag = DMA_FLAG_HTIF6;
        break;

    case 7:
        dma_flag = DMA_FLAG_HTIF7;
        break;

    default:
        // nothing to do here
        break;
    }

    return dma_flag;
}

uint32_t Dma::getTCFlagForStream(const uint32_t& DMAy_Streamx)
{
    // get the bits indicating the stream and devide by the bit offset between the streams
    const uint32_t stream = (DMAy_Streamx & 0xFF) / 0x18;
    uint32_t dma_flag = 0;

    switch (stream) {
    case 0:
        dma_flag = DMA_FLAG_TCIF0;
        break;

    case 1:
        dma_flag = DMA_FLAG_TCIF1;
        break;

    case 2:
        dma_flag = DMA_FLAG_TCIF2;
        break;

    case 3:
        dma_flag = DMA_FLAG_TCIF3;
        break;

    case 4:
        dma_flag = DMA_FLAG_TCIF4;
        break;

    case 5:
        dma_flag = DMA_FLAG_TCIF5;
        break;

    case 6:
        dma_flag = DMA_FLAG_TCIF6;
        break;

    case 7:
        dma_flag = DMA_FLAG_TCIF7;
        break;

    default:
        // nothing to do here
        break;
    }

    return dma_flag;
}

Dma::SemaphoreArray Dma::TCInterruptSemaphores;
Dma::SemaphoreArray Dma::HTInterruptSemaphores;
Dma::SemaphoreArray Dma::TEInterruptSemaphores;
Dma::CallbackArray Dma::TCInterruptCallbacks;
Dma::CallbackArray Dma::HTInterruptCallbacks;
Dma::CallbackArray Dma::TEInterruptCallbacks;

constexpr const std::array<const Dma, Dma::Description::__ENUM__SIZE + 1> Factory<Dma>::Container;
constexpr const std::array<const uint32_t, 2> Factory<Dma>::Clocks;
