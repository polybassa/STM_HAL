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

#ifndef SOURCES_PMD_DMA_H_
#define SOURCES_PMD_DMA_H_

#include <cstdint>
#include <limits>
#include <array>
#include <functional>
#include "stm32f10x.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_rcc.h"
#include "Semaphore.h"
#include "hal_Factory.h"

extern "C" {
void DMA1_Channel1_IRQHandler(void);
void DMA1_Channel2_IRQHandler(void);
void DMA1_Channel3_IRQHandler(void);
void DMA1_Channel4_IRQHandler(void);
void DMA1_Channel5_IRQHandler(void);
void DMA1_Channel6_IRQHandler(void);
void DMA1_Channel7_IRQHandler(void);

void DMA2_Channel1_IRQHandler(void);
void DMA2_Channel2_IRQHandler(void);
void DMA2_Channel3_IRQHandler(void);
void DMA2_Channel4_IRQHandler(void);
void DMA2_Channel5_IRQHandler(void);
}

namespace hal
{
struct Dma {
#include "Dma_config.h"

    enum InterruptSource {
        TC, // Transfer Complete
        HT, // Half Transfer
        TE // Transfer Error
    };

    const enum Description mDescription;

    Dma() = delete;
    Dma(const Dma&) = delete;
    Dma(Dma&&) = default;
    Dma& operator=(const Dma&) = delete;
    Dma& operator=(Dma&&) = delete;

    void setupTransfer(uint8_t const* const data, const size_t length, const bool repeat = false) const;
    void setupSendSingleCharMultipleTimes(uint8_t const* const data, const size_t length) const;

    void enable(void) const;
    void disable(void) const;
    bool registerInterruptSemaphore(os::Semaphore* const semaphore, const InterruptSource) const;
    bool registerInterruptCallback(std::function<void(void)> callback, const InterruptSource) const;
    void unregisterInterruptSemaphore(const InterruptSource) const;
    void unregisterInterruptCallback(const InterruptSource) const;

    uint16_t getCurrentDataCounter(void) const;
    void setCurrentDataCounter(const uint16_t) const;

    void memcpy(void const* const dest, void const* const src, const size_t length) const;

    inline static void DMA_IRQHandler(const Dma&     peripherie,
                                      const uint32_t TCFlag,
                                      const uint32_t HTFlag,
                                      const uint32_t TEFlag);
    inline static void DMA_TCIRQHandler(const Dma& peripherie);
    inline static void DMA_HTIRQHandler(const Dma& peripherie);
    inline static void DMA_TEIRQHandler(const Dma& peripherie);

private:
    constexpr Dma(const enum Description& desc,
                  const uint32_t&         peripherie,
                  const DMA_InitTypeDef&  conf,
                  const uint32_t          interrupt = 0,
                  const IRQn_Type         interruptNumber = IRQn_Type::UsageFault_IRQn) :
        mDescription(desc),
        mPeripherie(peripherie),
        mConfiguration(conf),
        mDmaInterrupt(interrupt),
        mDmaIRQn(
                 interruptNumber) {}

    const uint32_t mPeripherie;
    const DMA_InitTypeDef mConfiguration;
    const uint32_t mDmaInterrupt;
    const IRQn_Type mDmaIRQn;

    void initialize(void) const;

    using SemaphoreArray = std::array<os::Semaphore*, Dma::__ENUM__SIZE>;
    inline static void DMA_IRQHandlerSemaphore(const Dma& peripherie, const SemaphoreArray&);
    using CallbackArray = std::array<std::function<void (void)>, Dma::__ENUM__SIZE>;
    inline static void DMA_IRQHandlerCallback(const Dma& peripherie, const CallbackArray&);

    static SemaphoreArray TCInterruptSemaphores; // Transfer Complete
    static SemaphoreArray HTInterruptSemaphores; // Half Transfer
    static SemaphoreArray TEInterruptSemaphores; // Transfer Error
    static CallbackArray TCInterruptCallbacks;
    static CallbackArray HTInterruptCallbacks;
    static CallbackArray TEInterruptCallbacks;

    friend class Factory<Dma>;
};

template<>
class Factory<Dma>
{
#include "Dma_config.h"

    static constexpr const std::array<const uint32_t, 2> Clocks =
    { {
          RCC_AHBPeriph_DMA1, RCC_AHBPeriph_DMA2
      } };

    Factory(void)
    {
        for (const auto& clock : Clocks) {
            RCC_AHBPeriphClockCmd(clock, ENABLE);
        }

        for (const auto& dma : Container) {
            if (dma.mDescription != Dma::__ENUM__SIZE) {
                dma.initialize();
            }
        }
    }

    template<uint32_t peripherieBase, enum Dma::Description index>
    static constexpr const Dma& getByPeripherie(void)
    {
        return (Container[index]).mPeripherie ==
               peripherieBase ? Container[index] : getByPeripherie<peripherieBase,
                                                                   static_cast<enum Dma::Description>(index - 1)>();
    }

public:

    template<enum Dma::Description index>
    static constexpr const Dma& get(void)
    {
        static_assert(IS_DMA_ALL_PERIPH_BASE(Container[index].mPeripherie), "Invalid Peripheries ");
        static_assert(IS_DMA_DIR(Container[index].mConfiguration.DMA_DIR), "Invalid DIR");
        static_assert(IS_DMA_PERIPHERAL_INC_STATE(
                                                  Container[index].mConfiguration.DMA_PeripheralInc),
                      "Invalid PeripheralInc");
        static_assert(IS_DMA_MEMORY_INC_STATE(Container[index].mConfiguration.DMA_MemoryInc), "Invalid MemoryInc");
        static_assert(IS_DMA_MODE(Container[index].mConfiguration.DMA_Mode), "Invalid DMA MODE");
        static_assert(IS_DMA_PRIORITY(Container[index].mConfiguration.DMA_Priority), "Invalid Priority");
        static_assert(IS_DMA_M2M_STATE(Container[index].mConfiguration.DMA_M2M), "Invalid M2M state");
        static_assert(Container[index].mDmaInterrupt != 0 || Container[index].mDmaIRQn == IRQn::UsageFault_IRQn,
                      "Invalid IRQn");

        static_assert(index != Dma::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(index < Container[index + 1].mDescription, "Incorrect order of Dmas in DmaFactory");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<uint32_t peripherieBase>
    static constexpr const Dma& getByPeripherie(void)
    {
        static_assert(IS_DMA_ALL_PERIPH_BASE(peripherieBase), "Invalid Peripheries ");
        return getByPeripherie<peripherieBase,
                               static_cast<enum Dma::Description>(Dma::Description::__ENUM__SIZE - 1)>();
    }

    template<typename U>
    friend const U& getFactory(void);

    friend struct Dma;
};
}

#endif /* SOURCES_PMD_DMA_H_ */
