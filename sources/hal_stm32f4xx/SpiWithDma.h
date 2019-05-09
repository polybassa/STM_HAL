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

#ifndef SOURCES_PMD_SPIWITHDMA_H_
#define SOURCES_PMD_SPIWITHDMA_H_

#include <cstdint>
#include <array>
#include "Dma.h"
#include "Spi.h"
#include "Semaphore.h"
#include "hal_Factory.h"

namespace hal
{
struct SpiWithDma {
    SpiWithDma() = delete;
    SpiWithDma(const SpiWithDma&) = delete;
    SpiWithDma(SpiWithDma&&) = default;
    SpiWithDma& operator=(const SpiWithDma&) = delete;
    SpiWithDma& operator=(SpiWithDma&&) = delete;

    template<size_t n>
    size_t receive(std::array<uint8_t, n>&) const;
    size_t receive(uint8_t* const, const size_t) const;

    template<size_t n>
    size_t send(const std::array<uint8_t, n>&) const;
    size_t send(uint8_t const* const, const size_t) const;

private:
    constexpr SpiWithDma(Spi const* const spiInterface = nullptr,
                         const uint16_t&  dmaCmd = 0,
                         Dma const* const txDma = nullptr,
                         Dma const* const rxDma = nullptr) :
        mSpi(spiInterface), mDmaCmd(dmaCmd),
        mTxDma(txDma), mRxDma(rxDma) {}

    Spi const* const mSpi;
    const uint16_t mDmaCmd;
    Dma const* const mTxDma;
    Dma const* const mRxDma;

    void initialize(void) const;
    void registerInterruptCallbacks(void) const;

    bool isReadyToSend(void) const;
    bool isReadyToReceive(void) const;

    static constexpr const size_t MIN_LENGTH_FOR_DMA_TRANSFER = 2;
    static std::array<os::Semaphore, Spi::__ENUM__SIZE> DmaTransferCompleteSemaphores;

    friend class Factory<SpiWithDma>;
    friend class Dma;
};

template<size_t n>
size_t SpiWithDma::receive(std::array<uint8_t, n>& rx) const
{
    return receive(rx.data(), rx.size());
}

template<size_t n>
size_t SpiWithDma::send(const std::array<uint8_t, n>& tx) const
{
    return send(tx.data(), tx.size());
}

template<>
class Factory<SpiWithDma>
{
#include "SpiWithDma_config.h"

    Factory(void)
    {
//        for (auto & semaphore : SpiWithDma::DmaTransferCompleteSemaphores) {
//              if (!semaphore) {
//				os::Semaphore tmpSemahore;
//				semaphore = std::move(tmpSemahore);
//              }
//        }

        for (const auto& spi : Container) {
            spi.initialize();
        }
    }

public:
    template<enum Spi::Description index>
    static constexpr const SpiWithDma& get(void)
    {
        static_assert((Container[index].mSpi != nullptr), "Spi Interface not assigned");
        static_assert((Container[index].mDmaCmd != SPI_I2S_DMAReq_Tx) || (Container[index].mTxDma != nullptr),
                      "Tx Dma not assigned");
        static_assert((Container[index].mDmaCmd != SPI_I2S_DMAReq_Rx) || (Container[index].mRxDma != nullptr),
                      "Rx Dma not assigned");
        static_assert(index != Spi::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mSpi->mDescription == index,
                      "Wrong mapping between Description and Container. Use identical mapping as in Spi");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_SPIWITHDMA_H_ */
