// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

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
    if (mUsart.isInitalized() == false) {
        Trace(ZONE_ERROR, "Initalize Usart first!");
    }

    if (!IS_USART_DMAREQ(mDmaCmd)) {
        return;
    }
    USART_DMACmd(reinterpret_cast<USART_TypeDef*>(mUsart.mPeripherie), mDmaCmd, ENABLE);

    if (!DmaTransferCompleteSemaphores[static_cast<size_t>(mUsart.mDescription)] ||
        !DmaReceiveCompleteSemaphores[static_cast<size_t>(mUsart.mDescription)])
    {
        Trace(ZONE_ERROR, "Semaphore allocation failed/r/n");
    } else {
        registerInterruptSemaphores();
    }
}

void UsartWithDma::registerInterruptSemaphores(void) const
{
    if ((mTxDma != nullptr)) {
        mTxDma->registerInterruptSemaphore(&DmaTransferCompleteSemaphores.at(mUsart.mDescription),
                                           Dma::InterruptSource::TC);
    }

    if ((mRxDma != nullptr)) {
        mRxDma->registerInterruptSemaphore(&DmaReceiveCompleteSemaphores.at(mUsart.mDescription),
                                           Dma::InterruptSource::TC);
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

size_t UsartWithDma::send(uint8_t const* const data, const size_t length, const uint32_t ticksToWait) const
{
    if (data == nullptr) {
        return 0;
    }

    if ((mTxDma != nullptr) && (mDmaCmd & USART_DMAReq_Tx) && (length > MIN_LENGTH_FOR_DMA_TRANSFER)) {
        // clear Semaphore
        DmaTransferCompleteSemaphores.at(mUsart.mDescription).take(std::chrono::milliseconds(0));
        // we have DMA support
        mTxDma->setupTransfer(data, length);
        mTxDma->enable();

        if (DmaTransferCompleteSemaphores.at(mUsart.mDescription).take(std::chrono::milliseconds(ticksToWait))) {
            mTxDma->disable();
            return length;
        } else {
            mTxDma->disable();
            return 0;
        }
    } else {
        return mUsart.send(data, length);
    }
}

void UsartWithDma::receiveTimeoutCallback(void) const
{
    DmaReceiveCompleteSemaphores.at(mUsart.mDescription).giveFromISR();
}

void UsartWithDma::enableReceiveTimeout(const size_t bitsUntilTimeout) const
{
    if (mDmaCmd & USART_DMAReq_Rx) {
        mUsart.enableReceiveTimeout([this] {
            receiveTimeoutCallback();
        }, bitsUntilTimeout);
    }
}

void UsartWithDma::disableReceiveTimeout(void) const
{
    mUsart.disableReceiveTimeout();
}

size_t UsartWithDma::receiveWithTimeout(uint8_t* const data, const size_t length, const uint32_t ticksToWait) const
{
    /* Timeout is used to detect the end of a block of data */
    mUsart.enableReceiveTimeoutIT_Flag();
    const auto retVal = receive(data, length, ticksToWait);
    mUsart.disableReceiveTimeoutIT_Flag();
    return retVal;
}

size_t UsartWithDma::receive(uint8_t* const data, const size_t length, const uint32_t ticksToWait) const
{
    if (data == nullptr) {
        return 0;
    }

    if (mUsart.hasOverRunError()) {
        mUsart.clearOverRunError();
        //Trace(ZONE_INFO, "OverRun Error detected \r\n");
    }

    if ((mRxDma != nullptr) && (mDmaCmd & USART_DMAReq_Rx) && (length > MIN_LENGTH_FOR_DMA_TRANSFER)) {
        // clear Semaphore
        DmaReceiveCompleteSemaphores.at(mUsart.mDescription).take(std::chrono::milliseconds(0));
        // we have DMA support
        mRxDma->setupTransfer(data, length);
        mRxDma->enable();

        DmaReceiveCompleteSemaphores.at(mUsart.mDescription).take(std::chrono::milliseconds(ticksToWait));

        mRxDma->disable();
        return length - mRxDma->getCurrentDataCounter();
    } else {
        return mUsart.receive(data, length);
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

    if (mUsart.hasOverRunError()) {
        mUsart.clearOverRunError();
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

constexpr const std::array<const UsartWithDma, 1> Factory<UsartWithDma>::Container;
