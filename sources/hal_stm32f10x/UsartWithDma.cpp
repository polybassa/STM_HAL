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
        Trace(ZONE_ERROR, "Semaphore allocation failed\r\n");
    } else {
        registerInterruptSemaphores();
    }
    mInitialized = true;
    Trace(ZONE_INFO, "Init done!\r\n");
}

bool UsartWithDma::isReadyToReceive(void) const
{
    return mUsart.isReadyToReceive();
}

bool UsartWithDma::isReadyToSend(void) const
{
    return mUsart.isReadyToSend();
}

bool UsartWithDma::isInitalized(void) const
{
    return mInitialized;
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

size_t UsartWithDma::send(std::string_view str, const uint32_t ticksToWait) const
{
    return send(reinterpret_cast<uint8_t const* const>(str.data()), str.length(), ticksToWait);
}

size_t UsartWithDma::send(uint8_t const* const data, const size_t length, const uint32_t ticksToWait) const
{
    if (data == nullptr) {
        return 0;
    }

    const bool dmaSupport = (mTxDma != nullptr) && (mDmaCmd & USART_DMAReq_Tx);

    if (dmaSupport && (length > MIN_LENGTH_FOR_DMA_TRANSFER)) {
        // clear Semaphore
        DmaTransferCompleteSemaphores.at(mUsart.mDescription).take(std::chrono::milliseconds(0));
        // we have DMA support
        mTxDma->setupTransfer(data, length);
        mTxDma->enable();

        if (DmaTransferCompleteSemaphores.at(mUsart.mDescription).take(std::chrono::milliseconds(ticksToWait))) {
            mTxDma->disable();
            return length;
        } else {
            Trace(ZONE_INFO, "Failed\r\n");

            mTxDma->disable();
            return 0;
        }
    } else {
        return mUsart.send(data, length);
    }
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

    const bool dmaSupport = (mRxDma != nullptr) && (mDmaCmd & USART_DMAReq_Rx);

    if (dmaSupport && (length > MIN_LENGTH_FOR_DMA_TRANSFER)) {
        // clear Semaphore
        DmaReceiveCompleteSemaphores.at(mUsart.mDescription).take(std::chrono::milliseconds(0));
        mRxDma->setupTransfer(data, length);
        mRxDma->enable();

        DmaReceiveCompleteSemaphores.at(mUsart.mDescription).take(std::chrono::milliseconds(ticksToWait));

        mRxDma->disable();
        return length - mRxDma->getCurrentDataCounter();
    } else {
        return mUsart.receive(data, length);
    }
}

size_t UsartWithDma::receiveAvailableData(uint8_t* const data, const size_t length) const
{
    //TODO refactor
    return mUsart.receiveAvailableData(data, length);
}

void UsartWithDma::sendNonBlocking(uint8_t const* const data, const size_t length, const bool repeat) const
{
    if (data == nullptr) {
        return;
    }

    const bool dmaSupport = (mTxDma != nullptr) && (mDmaCmd & USART_DMAReq_Tx);

    if (dmaSupport) {
        mTxDma->setupTransfer(data, length, repeat);
        mTxDma->enable();
    }
}
void UsartWithDma::receiveNonBlocking(uint8_t const* const data, const size_t length, const bool repeat) const
{
    if (data == nullptr) {
        return;
    }

    const bool dmaSupport = (mRxDma != nullptr) && (mDmaCmd & USART_DMAReq_Rx);

    if (dmaSupport) {
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

constexpr const std::array<const UsartWithDma, Factory<UsartWithDma>::CONTAINERSIZE> Factory<UsartWithDma>::Container;
