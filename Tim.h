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

#ifndef SOURCES_PMD_TIM_H_
#define SOURCES_PMD_TIM_H_

#include <cstdint>
#include <limits>
#include <array>
#include "stm32f4xx_tim.h"
#include "stm32f4xx_rcc.h"
#include "hal_Factory.h"

// ================================================================================================
#define IS_TIM_ALL_PERIPH_BASE(PERIPH) ( \
                                        ((PERIPH) == TIM1_BASE) || \
                                        ((PERIPH) == TIM2_BASE) || \
                                        ((PERIPH) == TIM3_BASE) || \
                                        ((PERIPH) == TIM4_BASE) || \
                                        ((PERIPH) == TIM5_BASE) || \
                                        ((PERIPH) == TIM6_BASE) || \
                                        ((PERIPH) == TIM7_BASE) || \
                                        ((PERIPH) == TIM8_BASE) || \
                                        ((PERIPH) == TIM9_BASE) || \
                                        ((PERIPH) == TIM10_BASE) || \
                                        ((PERIPH) == TIM11_BASE) || \
                                        ((PERIPH) == TIM12_BASE) || \
                                        ((PERIPH) == TIM13_BASE) || \
                                        ((PERIPH) == TIM14_BASE))
// ================================================================================================

namespace dev
{
struct SensorBLDC;
}

namespace hal
{
struct Tim {
#include "Tim_config.h"

    Tim() = delete;
    Tim(const Tim&) = delete;
    Tim(Tim &&) = default;
    Tim& operator=(const Tim&) = delete;
    Tim& operator=(Tim &&) = delete;

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

    const enum Description mDescription;

private:
    constexpr Tim(const enum Description&        desc,
                  const uint32_t&                peripherie,
                  const TIM_TimeBaseInitTypeDef& conf) :
        mDescription(desc),
        mPeripherie(peripherie),
        mConfiguration(conf) {}

    const uint32_t mPeripherie;
    const TIM_TimeBaseInitTypeDef mConfiguration;
    mutable uint32_t mClockFrequency = 0;

    void initialize(void) const;
    TIM_TypeDef* getBasePointer(void) const;

    friend class Factory<Tim>;
    friend struct Pwm;
    friend struct HallDecoder;
    friend struct HalfBridge;
    friend struct HallMeter;
    friend struct dev::SensorBLDC;
    friend struct PhaseCurrentSensor;
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
                (clock == RCC_APB1Periph_TIM5) ||
                (clock == RCC_APB1Periph_TIM6) ||
                (clock == RCC_APB1Periph_TIM7) ||
                (clock == RCC_APB1Periph_TIM12) ||
                (clock == RCC_APB1Periph_TIM13) ||
                (clock == RCC_APB1Periph_TIM14))
            {
                RCC_APB1PeriphClockCmd(clock, ENABLE);
            } else if ((clock == RCC_APB2Periph_TIM1) ||
                       (clock == RCC_APB2Periph_TIM8) ||
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

                tim.mClockFrequency = clocks.SYSCLK_Frequency;
            }
        }
    }
public:

    template<enum Tim::Description index>
    static constexpr const Tim& get(void)
    {
        static_assert(IS_TIM_ALL_PERIPH_BASE(Container[index].mPeripherie), "Invalid Peripheries ");
        static_assert(IS_TIM_CKD_DIV(Container[index].mConfiguration.TIM_ClockDivision), "Invalid Parameter");
        static_assert(IS_TIM_COUNTER_MODE(Container[index].mConfiguration.TIM_CounterMode), "Invalid Parameter");

        static_assert(index != Tim::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(index < Container[index + 1].mDescription, "Incorrect order of Tims in TimFactory");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        // configuration
        static_assert(Container[index].mConfiguration.TIM_Prescaler >= 0x0000, "TIM Period under 0x0000");
        static_assert(Container[index].mConfiguration.TIM_Prescaler <= 0xFFFF, "TIM Period over 0xFFFF");
        static_assert(Container[index].mConfiguration.TIM_Period >= 0x0000, "TIM Period under 0x0000");
        static_assert(Container[index].mConfiguration.TIM_Period <= 0xFFFF, "TIM Period over 0xFFFF");
        static_assert(Container[index].mConfiguration.TIM_RepetitionCounter >= 0x00, "TIM Period under 0x00");
        static_assert(Container[index].mConfiguration.TIM_RepetitionCounter <= 0xFF, "TIM Period over 0xFF");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_TIM_H_ */
