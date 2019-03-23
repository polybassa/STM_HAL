// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_TIM_H_
#define SOURCES_PMD_TIM_H_

#include <cstdint>
#include <limits>
#include <array>
#include <functional>

#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "hal_Factory.h"

namespace hal
{
struct Tim {
#include "Tim_config.h"

    Tim() = delete;
    Tim(const Tim&) = delete;
    Tim(Tim&&) = default;
    Tim& operator=(const Tim&) = delete;
    Tim& operator=(Tim&&) = delete;

    void selectOnePulseMode(const uint16_t mode) const;
    void setCounterValue(const uint32_t) const;
    void setAutoReloadValue(const uint32_t) const;
    uint32_t getCounterValue(void) const;
    uint32_t getTimerFrequency(void) const;
    uint32_t getPeriode(void) const;
    void setPeriode(const uint32_t period) const;
    void enable(void) const;
    void disable(void) const;
    void selectOutputTrigger(uint16_t TRGO_Source) const;
    ITStatus getInterruptStatus(const uint16_t interruptFlag) const;
    void clearPendingInterruptFlag(const uint16_t interruptFlag) const;

    void registerInterruptCallback(std::function<void(void)> ) const;
    void unregisterInterruptCallback(void) const;

    inline static void TIM_IRQHandler(const Tim& peripherie);
    inline static void TIM_IRQHandlerCallback(const Tim& tim);

    const enum Description mDescription;

private:
    constexpr Tim(const enum Description&        desc,
                  const uint32_t&                peripherie,
                  const TIM_TimeBaseInitTypeDef& conf,
                  const uint16_t                 interrupt = 0,
                  const IRQn_Type                irq = IRQn::UsageFault_IRQn) :
        mDescription(desc), mPeripherie(peripherie),
        mConfiguration(conf), mInterrupt(interrupt), mIrq(irq) {}

    const uint32_t mPeripherie;
    const TIM_TimeBaseInitTypeDef mConfiguration;
    mutable uint32_t mClockFrequency = 0;
    const uint16_t mInterrupt;
    const IRQn_Type mIrq;

    void initialize(void) const;
    TIM_TypeDef* getBasePointer(void) const;

    using CallbackArray = std::array<std::function<void (void)>, Tim::__ENUM__SIZE>;
    static CallbackArray TimCallbacks;

    friend class Factory<Tim>;
};

template<>
class Factory<Tim>
{
#include "Tim_config.h"

    Factory(void)
    {
        for (const auto& clock : Clocks) {
            if ((clock == RCC_APB1Periph_TIM2) ||
                (clock == RCC_APB1Periph_TIM3) ||
                (clock == RCC_APB1Periph_TIM4) ||
                (clock == RCC_APB1Periph_TIM6) ||
                (clock == RCC_APB1Periph_TIM7))
            {
                RCC_APB1PeriphClockCmd(clock, ENABLE);
            }
            if ((clock == RCC_APB2Periph_TIM1) ||
                (clock == RCC_APB2Periph_TIM8) ||
                (clock == RCC_APB2Periph_TIM15) ||
                (clock == RCC_APB2Periph_TIM16) ||
                (clock == RCC_APB2Periph_TIM17) ||
                (clock == RCC_APB2Periph_TIM9) ||
                (clock == RCC_APB2Periph_TIM10) ||
                (clock == RCC_APB2Periph_TIM11))
            {
                RCC_APB2PeriphClockCmd(clock, ENABLE);
            }
        }

        RCC_ClocksTypeDef clocks;
        RCC_GetClocksFreq(&clocks);

        for (const auto& tim : Container) {
            if (tim.mDescription != Tim::__ENUM__SIZE) {
                tim.initialize();

                switch (tim.mPeripherie) {
                case TIM1_BASE:
                    tim.mClockFrequency = clocks.PCLK2_Frequency;
                    break;

                case TIM2_BASE:
                    tim.mClockFrequency = clocks.PCLK1_Frequency;
                    break;

                case TIM3_BASE:
                    tim.mClockFrequency = clocks.PCLK1_Frequency;
                    break;

                case TIM4_BASE: // TODO validate frequency
                    tim.mClockFrequency = clocks.PCLK1_Frequency;
                    break;

                case TIM6_BASE: // TODO validate frequency
                    tim.mClockFrequency = clocks.PCLK1_Frequency;
                    break;

                case TIM7_BASE: // TODO validate frequency
                    tim.mClockFrequency = clocks.PCLK1_Frequency;
                    break;

                case TIM8_BASE:
                    tim.mClockFrequency = clocks.PCLK2_Frequency;
                    break;

                case TIM15_BASE:
                    tim.mClockFrequency = clocks.PCLK2_Frequency;
                    break;

                case TIM16_BASE:
                    tim.mClockFrequency = clocks.PCLK2_Frequency;
                    break;

                case TIM17_BASE:
                    tim.mClockFrequency = clocks.PCLK2_Frequency;
                    break;

                case TIM9_BASE:
                    tim.mClockFrequency = clocks.PCLK2_Frequency;
                    break;

                case TIM10_BASE:
                    tim.mClockFrequency = clocks.PCLK2_Frequency;
                    break;

                case TIM11_BASE:
                    tim.mClockFrequency = clocks.PCLK2_Frequency;
                    break;

                default:
                    tim.mClockFrequency = 0;
                }
            }
        }
    }
    template<uint32_t peripherieBase, enum Tim::Description index>
    static constexpr const Tim& getByPeripherie(void)
    {
        return (Container[index]).mPeripherie ==
               peripherieBase ? Container[index] : getByPeripherie<peripherieBase,
                                                                   static_cast<enum Tim::Description>(index - 1)>();
    }

public:

    template<enum Tim::Description index>
    static constexpr const Tim& get(void)
    {
        static_assert(IS_TIM_ALL_PERIPH_BASE(Container[index].mPeripherie), "Invalid Peripheries ");
        static_assert(IS_TIM_CKD_DIV(Container[index].mConfiguration.TIM_ClockDivision), "Invalid Parameter");
        static_assert(IS_TIM_COUNTER_MODE(Container[index].mConfiguration.TIM_CounterMode), "Invalid Parameter");
        static_assert(IS_TIM_IT(Container[index].mInterrupt) || ((Container[index].mInterrupt == 0) &&
                                                                 (Container[index].mIrq == IRQn::UsageFault_IRQn)),
                      "Invalid IRQn");
        static_assert(index != Tim::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(index < Container[index + 1].mDescription, "Incorrect order of Tims in TimFactory");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<uint32_t peripherieBase>
    static constexpr const Tim& getByPeripherie(void)
    {
        static_assert(IS_TIM_ALL_PERIPH_BASE(peripherieBase), "Invalid Peripheries ");
        return getByPeripherie<peripherieBase,
                               static_cast<enum Tim::Description>(Tim::Description::__ENUM__SIZE - 1)>();
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_TIM_H_ */
