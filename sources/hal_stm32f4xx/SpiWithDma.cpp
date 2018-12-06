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

#include "SpiWithDma.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR |
                                                        ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;
using hal::Dma;
using hal::Factory;
using hal::Spi;
using hal::SpiWithDma;

std::array<os::Semaphore, Spi::__ENUM__SIZE> SpiWithDma::DmaTransferCompleteSemaphores;

void SpiWithDma::initialize() const
{
    if (!IS_SPI_I2S_DMAREQ(mDmaCmd)) {
        return;
    }
    SPI_I2S_DMACmd(reinterpret_cast<SPI_TypeDef*>(mSpi->mPeripherie), mDmaCmd, ENABLE);

    if (!DmaTransferCompleteSemaphores[(size_t)mSpi->mDescription]) {
        Trace(ZONE_ERROR, "Semaphore allocation failed\r\n");
    } else {
        registerInterruptCallbacks();
    }
}

void SpiWithDma::registerInterruptCallbacks(void) const
{
    if (mTxDma) {
        mTxDma->registerInterruptSemaphore(&DmaTransferCompleteSemaphores.at(mSpi->mDescription),
                                           Dma::InterruptSource::TC);
    }

    if (mRxDma) {
        mRxDma->registerInterruptSemaphore(&DmaTransferCompleteSemaphores.at(mSpi->mDescription),
                                           Dma::InterruptSource::TC);
    }
}

size_t SpiWithDma::send(uint8_t const* const data, const size_t length) const
{
    if (data == nullptr) {
        return 0;
    }

    if (mTxDma && (mDmaCmd & SPI_I2S_DMAReq_Tx)
        && (length > MIN_LENGTH_FOR_DMA_TRANSFER))
    {
        // clear Semaphore
        DmaTransferCompleteSemaphores.at(mSpi->mDescription).take(std::chrono::microseconds(1));
        // we have DMA support
//        mTxDma->setupTransfer((uint8_t* const)data, length);
        mTxDma->setupTransfer(data, length, false);
        mTxDma->enable();

        DmaTransferCompleteSemaphores.at(mSpi->mDescription).take();

        mTxDma->disable();
        return length;
    } else {
        return mSpi->send(data, length);
    }
}

bool SpiWithDma::isReadyToReceive(void) const
{
    return mSpi->isReadyToReceive();
}

size_t SpiWithDma::receive(uint8_t* const data, const size_t length) const
{
    if (data == nullptr) {
        return 0;
    }

    if (mRxDma && (mDmaCmd & SPI_I2S_DMAReq_Rx)
        && (length > MIN_LENGTH_FOR_DMA_TRANSFER) && (mTxDma && (mDmaCmd & SPI_I2S_DMAReq_Tx)))
    {
        // we have DMA support
        // disable TX interrupt to avoid two callback calls
        mTxDma->unregisterInterruptSemaphore(Dma::InterruptSource::TC);

        mRxDma->setupTransfer((uint8_t* const)data, length);
        const uint8_t dummy = 0xff;
        mTxDma->setupSendSingleCharMultipleTimes(&dummy, length);

        mRxDma->enable();
        mTxDma->enable();

        DmaTransferCompleteSemaphores.at(mSpi->mDescription).take();

        mTxDma->disable();
        mRxDma->disable();

        // reregister TX Callback
        registerInterruptCallbacks();
        return length;
    } else {
        return mSpi->receive(data, length);
    }
}

bool SpiWithDma::isReadyToSend(void) const
{
    return mSpi->isReadyToSend();
}

constexpr const std::array<const SpiWithDma, Spi::__ENUM__SIZE> Factory<SpiWithDma>::Container;
