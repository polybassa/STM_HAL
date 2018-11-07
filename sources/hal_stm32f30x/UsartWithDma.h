// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_USARTWITHDMA_H_
#define SOURCES_PMD_USARTWITHDMA_H_

#include <cstdint>
#include <array>
#include "Dma.h"
#include "Usart.h"
#include "Semaphore.h"
#include "hal_Factory.h"

namespace hal
{
struct UsartWithDma {
    UsartWithDma() = delete;
    UsartWithDma(const UsartWithDma&) = delete;
    UsartWithDma(UsartWithDma&&) = default;
    UsartWithDma& operator=(const UsartWithDma&) = delete;
    UsartWithDma& operator=(UsartWithDma&&) = delete;

    template<size_t n>
    size_t send(const std::array<uint8_t, n>&) const;
    size_t send(uint8_t const* const, const size_t, const uint32_t ticksToWait = portMAX_DELAY) const;

    template<size_t n>
    size_t receive(std::array<uint8_t, n>&) const;
    size_t receive(uint8_t* const, const size_t, const uint32_t ticksToWait = portMAX_DELAY) const;

    size_t receiveWithTimeout(uint8_t* const data, const size_t length, const uint32_t ticksToWait =
                                  portMAX_DELAY) const;

    template<size_t n>
    void sendNonBlocking(const std::array<uint8_t, n>& rx, const bool repeat) const;
    template<size_t n>
    void receiveNonBlocking(const std::array<uint8_t, n>& rx, const bool repeat) const;

    void sendNonBlocking(uint8_t const* const, const size_t, const bool repeat) const;
    void receiveNonBlocking(uint8_t const* const, const size_t, const bool repeat) const;

    void stopNonBlockingSend(void) const;
    void stopNonBlockingReceive(void) const;

    void registerTransferCompleteCallback(std::function<void(void)> ) const;
    void registerReceiveCompleteCallback(std::function<void(void)> ) const;

    void enableReceiveTimeout(const size_t bitsUntilTimeout) const;
    void disableReceiveTimeout(void) const;

    const Usart& mUsart;

private:
    constexpr UsartWithDma(const Usart&     usartInterface,
                           const uint16_t&  dmaCmd = 0,
                           Dma const* const txDma = nullptr,
                           Dma const* const rxDma = nullptr) :
        mUsart(usartInterface), mDmaCmd(dmaCmd), mTxDma(txDma),
        mRxDma(rxDma) {}

    const uint16_t mDmaCmd;
    Dma const* const mTxDma;
    Dma const* const mRxDma;

    void initialize(void) const;
    void registerInterruptSemaphores(void) const;
    void receiveTimeoutCallback(void) const;

    static constexpr const size_t MIN_LENGTH_FOR_DMA_TRANSFER = 0;
    static std::array<os::Semaphore, Usart::__ENUM__SIZE> DmaTransferCompleteSemaphores;
    static std::array<os::Semaphore, Usart::__ENUM__SIZE> DmaReceiveCompleteSemaphores;

    friend class Factory<UsartWithDma>;
    friend struct Dma;
};

template<size_t n>
size_t UsartWithDma::send(const std::array<uint8_t, n>& tx) const
{
    return send(tx.data(), tx.size());
}

template<size_t n>
size_t UsartWithDma::receive(std::array<uint8_t, n>& rx) const
{
    return receive(rx.data(), rx.size());
}

template<size_t n>
void UsartWithDma::sendNonBlocking(const std::array<uint8_t, n>& tx, const bool repeat) const
{
    sendNonBlocking(tx.data(), tx.size(), repeat);
}

template<size_t n>
void UsartWithDma::receiveNonBlocking(const std::array<uint8_t, n>& rx, const bool repeat) const
{
    receiveNonBlocking(rx.data(), rx.size(), repeat);
}

template<>
class Factory<UsartWithDma>
{
#include "UsartWithDma_config.h"

    Factory(void)
    {
        for (const auto& usart : Container) {
            usart.initialize();
        }
    }

    template<Usart::Description desc>
    static constexpr const UsartWithDma& _get(const size_t i)
    {
        return Container[i].mUsart.mDescription == desc ? (Container[i]) : _get<desc>(i + 1);
    }

public:

    template<enum Usart::Description desc>
    static constexpr const UsartWithDma& get(void)
    {
        static_assert((_get<desc>(0).mDmaCmd != USART_DMAReq_Tx) ||
                      (_get<desc>(0).mTxDma != nullptr), "Tx Dma not assigned");
        static_assert((_get<desc>(0).mDmaCmd != USART_DMAReq_Rx) ||
                      (_get<desc>(0).mRxDma != nullptr), "Rx Dma not assigned");

        return _get<desc>(0);
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_USARTWITHDMA_H_ */
