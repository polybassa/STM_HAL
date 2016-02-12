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

#include "UsartWithDma.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Dma;
using hal::Factory;
using hal::Usart;
using hal::UsartWithDma;

std::array<os::Semaphore, Usart::__ENUM__SIZE> UsartWithDma::DmaTransferCompleteSemaphores;
std::array<os::Semaphore, Usart::__ENUM__SIZE> UsartWithDma::DmaReceiveCompleteSemaphores;

void UsartWithDma::initialize() const
{
    if (!IS_USART_DMAREQ(mDmaCmd) || (mUsart == nullptr)) {
        return;
    }
    USART_Cmd(reinterpret_cast<USART_TypeDef*>(mUsart->mPeripherie), DISABLE);
    USART_DMACmd(reinterpret_cast<USART_TypeDef*>(mUsart->mPeripherie), mDmaCmd, ENABLE);
    USART_Cmd(reinterpret_cast<USART_TypeDef*>(mUsart->mPeripherie), ENABLE);

    if (!DmaTransferCompleteSemaphores[static_cast<size_t>(mUsart->mDescription)] ||
        !DmaReceiveCompleteSemaphores[static_cast<size_t>(mUsart->mDescription)])
    {
        Trace(ZONE_ERROR, "Semaphore allocation failed/r/n");
    } else {
        registerInterruptSemaphores();
    }
}

void UsartWithDma::registerInterruptSemaphores(void) const
{
    if ((mTxDma != nullptr)) {
        mTxDma->registerInterruptSemaphore(&DmaTransferCompleteSemaphores.at(
                                               mUsart->mDescription), Dma::InterruptSource::TC);
    }

    if ((mRxDma != nullptr)) {
        mRxDma->registerInterruptSemaphore(&DmaReceiveCompleteSemaphores.at(
                                               mUsart->mDescription), Dma::InterruptSource::TC);
    }
}

void UsartWithDma::registerTransferCompleteCallback(std::function<void(void)> f) const
{
    if (mTxDma != nullptr) {
        mTxDma->registerInterruptCallback(f, Dma::InterruptSource::TC);
    }
}

void UsartWithDma::registerReceiveCompleteCallback(std::function<void(void)> f) const
{
    if (mRxDma != nullptr) {
        mRxDma->registerInterruptCallback(f, Dma::InterruptSource::TC);
    }
}

size_t UsartWithDma::getNonBlockingSendDataCounter(void) const
{
    if (mTxDma != nullptr) {
        return mTxDma->getCurrentDataCounter();
    }
    return 0;
}

void UsartWithDma::setBaudRate(const size_t baudRate) const
{
    if (mUsart) {
        mUsart->setBaudRate(baudRate);
    }
}

size_t UsartWithDma::send(uint8_t const* const data, const size_t length, const uint32_t ticksToWait) const
{
    if (data == nullptr) {
        return 0;
    }

    if ((mTxDma != nullptr) && (mDmaCmd & USART_DMAReq_Tx) && (length > MIN_LENGTH_FOR_DMA_TRANSFER)) {
        // we have DMA support
        mTxDma->setupTransfer(data, length);
        mTxDma->enable();

        if (DmaTransferCompleteSemaphores.at(mUsart->mDescription).take(ticksToWait)) {
            mTxDma->disable();
            return length;
        } else {
            mTxDma->disable();
            return 0;
        }
    } else {
        return mUsart->send(data, length);
    }
}

size_t UsartWithDma::receive(uint8_t* const data, const size_t length, const uint32_t ticksToWait) const
{
    if (data == nullptr) {
        return 0;
    }

    if (mUsart->hasOverRunError()) {
        mUsart->clearOverRunError();
        //Trace(ZONE_INFO, "OverRun Error detected \r\n");
    }

    if ((mRxDma != nullptr) && (mDmaCmd & USART_DMAReq_Rx) && (length > MIN_LENGTH_FOR_DMA_TRANSFER)) {
        // we have DMA support
        mRxDma->setupTransfer(data, length);
        mRxDma->enable();

        if (DmaReceiveCompleteSemaphores.at(mUsart->mDescription).take(ticksToWait)) {
            mRxDma->disable();
            return length;
        } else {
            mRxDma->disable();
            return 0;
        }
    } else {
        return mUsart->receive(data, length);
    }
}

void UsartWithDma::sendNonBlocking(uint8_t const* const data, const size_t length, const bool repeat) const
{
    if (data == nullptr) {
        return;
    }

    if ((mTxDma != nullptr) && (mDmaCmd & USART_DMAReq_Tx)) {
        // we have DMA support
        mTxDma->setupTransfer(data, length, repeat);
        mTxDma->enable();
    }
}
void UsartWithDma::receiveNonBlocking(uint8_t const* const data, const size_t length, const bool repeat) const
{
    if (data == nullptr) {
        return;
    }

    if ((mRxDma != nullptr) && (mDmaCmd & USART_DMAReq_Rx)) {
        // we have DMA support
        mRxDma->setupTransfer(data, length, repeat);
        mRxDma->enable();
    }

    if (mUsart->hasOverRunError()) {
        mUsart->clearOverRunError();
        Trace(ZONE_INFO, "OverRun Error detected \r\n");
    }
}

void UsartWithDma::stopNonBlockingSend(void) const
{
    if (mTxDma != nullptr) {
        mTxDma->disable();
    }
}
void UsartWithDma::stopNonBlockingReceive(void) const
{
    if (mRxDma != nullptr) {
        mRxDma->disable();
    }
}

constexpr const std::array<const UsartWithDma, Usart::__ENUM__SIZE> Factory<UsartWithDma>::Container;
