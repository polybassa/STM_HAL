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
#include "stm32f4xx.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_rcc.h"
#include "Semaphore.h"
#include "hal_Factory.h"
#include "Nvic.h"

// ================================================================================================
#define IS_DMA_ALL_PERIPH_BASE(PERIPH) (((PERIPH) == DMA1_Stream0_BASE) || \
                                        ((PERIPH) == DMA1_Stream1_BASE) || \
                                        ((PERIPH) == DMA1_Stream2_BASE) || \
                                        ((PERIPH) == DMA1_Stream3_BASE) || \
                                        ((PERIPH) == DMA1_Stream4_BASE) || \
                                        ((PERIPH) == DMA1_Stream5_BASE) || \
                                        ((PERIPH) == DMA1_Stream6_BASE) || \
                                        ((PERIPH) == DMA1_Stream7_BASE) || \
                                        ((PERIPH) == DMA2_Stream0_BASE) || \
                                        ((PERIPH) == DMA2_Stream1_BASE) || \
                                        ((PERIPH) == DMA2_Stream2_BASE) || \
                                        ((PERIPH) == DMA2_Stream3_BASE) || \
                                        ((PERIPH) == DMA2_Stream4_BASE) || \
                                        ((PERIPH) == DMA2_Stream5_BASE) || \
                                        ((PERIPH) == DMA2_Stream6_BASE) || \
                                        ((PERIPH) == DMA2_Stream7_BASE))
// ================================================================================================

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
    Dma(Dma &&) = default;
    Dma& operator=(const Dma&) = delete;
    Dma& operator=(Dma &&) = delete;

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

    inline static void ClearInterruptFlag(const Dma* const peripherie);
    inline static bool GetInterruptFlagStatus(const Dma* const peripherie);
    inline static void DMA_IRQHandler(const Dma* const peripherie);
    inline static void DMA_TCIRQHandler(const Dma& peripherie);
    inline static void DMA_HTIRQHandler(const Dma& peripherie);
    inline static void DMA_TEIRQHandler(const Dma& peripherie);

    inline static uint32_t getITForStream(const uint32_t& DMAy_Streamx, const uint32_t& DMA_IT);

private:
    constexpr Dma(const enum Description& desc,
                  const uint32_t&         peripherie,
                  const DMA_InitTypeDef&  conf,
                  const uint32_t          interrupt = 0,
                  const Nvic*             nvic = nullptr) :
        mDescription(desc),
        mPeripherie(peripherie),
        mConfiguration(conf),
        mDmaInterrupt(interrupt),
        mpNvic(nvic) {}

    const uint32_t mPeripherie;
    const DMA_InitTypeDef mConfiguration;
    const uint32_t mDmaInterrupt;
    const Nvic* mpNvic;

    void initialize(void) const;

    using SemaphoreArray = std::array<os::Semaphore*, Dma::__ENUM__SIZE>;
    inline static void DMA_IRQHandlerSemaphore(const Dma& peripherie, const SemaphoreArray&);
    using CallbackArray = std::array<std::function<void(void)>, Dma::__ENUM__SIZE>;
    inline static void DMA_IRQHandlerCallback(const Dma& peripherie, const CallbackArray&);

    inline static uint32_t getFEFlagForStream(const uint32_t& DMAy_Streamx);
    inline static uint32_t getDMEFlagForStream(const uint32_t& DMAy_Streamx);
    inline static uint32_t getTEFlagForStream(const uint32_t& DMAy_Streamx);
    inline static uint32_t getHTFlagForStream(const uint32_t& DMAy_Streamx);
    inline static uint32_t getTCFlagForStream(const uint32_t& DMAy_Streamx);

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
          RCC_AHB1Periph_DMA1, RCC_AHB1Periph_DMA2
      } };

    Factory(void)
    {
        for (const auto& clock : Clocks) {
            RCC_AHB1PeriphClockCmd(clock, ENABLE);
        }

        RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_DMA1, ENABLE);
        RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_DMA2, ENABLE);
        RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_DMA1, DISABLE);
        RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_DMA2, DISABLE);

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
        static_assert(index != Dma::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(index < Container[index + 1].mDescription, "Incorrect order of Dmas in DmaFactory");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        static_assert(IS_DMA_ALL_PERIPH_BASE(Container[index].mPeripherie), "Invalid Peripheries ");
        static_assert(IS_DMA_CHANNEL(Container[index].mConfiguration.DMA_Channel), "Invalid Channel");
        static_assert(IS_DMA_DIRECTION(Container[index].mConfiguration.DMA_DIR), "Invalid DIR");
        static_assert(IS_DMA_BUFFER_SIZE(Container[index].mConfiguration.DMA_BufferSize), "Invalid Buffer Size");
        static_assert(IS_DMA_PERIPHERAL_INC_STATE(
                                                  Container[index].mConfiguration.DMA_PeripheralInc),
                      "Invalid PeripheralInc");
        static_assert(IS_DMA_MEMORY_INC_STATE(Container[index].mConfiguration.DMA_MemoryInc), "Invalid MemoryInc");
        static_assert(IS_DMA_PERIPHERAL_DATA_SIZE(
                                                  Container[index].mConfiguration.DMA_PeripheralDataSize),
                      "Invalid peripheral data size");
        static_assert(IS_DMA_MEMORY_DATA_SIZE(
                                              Container[index].mConfiguration.DMA_MemoryDataSize),
                      "Invalid memory data size");
        static_assert(IS_DMA_MODE(Container[index].mConfiguration.DMA_Mode), "Invalid DMA MODE");
        static_assert(IS_DMA_PRIORITY(Container[index].mConfiguration.DMA_Priority), "Invalid Priority");
        static_assert(IS_DMA_FIFO_MODE_STATE(Container[index].mConfiguration.DMA_FIFOMode), "Invalid FIFO Mode state");
        static_assert(IS_DMA_FIFO_THRESHOLD(
                                            Container[index].mConfiguration.DMA_FIFOThreshold),
                      "Invalid FIFO threshold");
        static_assert(IS_DMA_MEMORY_BURST(Container[index].mConfiguration.DMA_MemoryBurst), "Invalid memory burst");
        static_assert(IS_DMA_PERIPHERAL_BURST(
                                              Container[index].mConfiguration.DMA_PeripheralBurst),
                      "Invalid peripheral burst");

        static_assert(Container[index].mDmaInterrupt != 0 || Container[index].mpNvic == nullptr,
                      "Invalid Interrupt config (Nvic and DMA_IT)");

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
