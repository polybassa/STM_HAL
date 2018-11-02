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
#include "stm32f30x_tim.h"
#include "stm32f30x_rcc.h"
#include "hal_Factory.h"

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
    Tim(Tim&&) = default;
    Tim& operator=(const Tim&) = delete;
    Tim& operator=(Tim&&) = delete;

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
        mDescription(desc), mPeripherie(peripherie),
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
                (clock == RCC_APB1Periph_TIM6) ||
                (clock == RCC_APB1Periph_TIM7))
            {
                RCC_APB1PeriphClockCmd(clock, ENABLE);
            } else if ((clock == RCC_APB2Periph_TIM1) ||
                       (clock == RCC_APB2Periph_TIM8) ||
                       (clock == RCC_APB2Periph_TIM15) ||
                       (clock == RCC_APB2Periph_TIM16) ||
                       (clock == RCC_APB2Periph_TIM17) ||
                       (clock == RCC_APB2Periph_TIM20))
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
                    tim.mClockFrequency = clocks.TIM1CLK_Frequency;
                    break;

                case TIM2_BASE:
                    tim.mClockFrequency = clocks.TIM2CLK_Frequency;
                    break;

                case TIM3_BASE:
                    tim.mClockFrequency = clocks.TIM3CLK_Frequency;
                    break;

                case TIM4_BASE: // TODO validate frequency
                    tim.mClockFrequency = clocks.TIM3CLK_Frequency;
                    break;

                case TIM6_BASE: // TODO validate frequency
                    tim.mClockFrequency = clocks.TIM3CLK_Frequency;
                    break;

                case TIM7_BASE: // TODO validate frequency
                    tim.mClockFrequency = clocks.TIM3CLK_Frequency;
                    break;

                case TIM8_BASE:
                    tim.mClockFrequency = clocks.TIM8CLK_Frequency;
                    break;

                case TIM15_BASE:
                    tim.mClockFrequency = clocks.TIM15CLK_Frequency;
                    break;

                case TIM16_BASE:
                    tim.mClockFrequency = clocks.TIM16CLK_Frequency;
                    break;

                case TIM17_BASE:
                    tim.mClockFrequency = clocks.TIM17CLK_Frequency;
                    break;

                case TIM20_BASE:
                    tim.mClockFrequency = clocks.TIM20CLK_Frequency;
                    break;

                default:
                    tim.mClockFrequency = 0;
                }
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

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_TIM_H_ */
