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
    Tim(Tim &&) = default;
    Tim& operator=(const Tim&) = delete;
    Tim& operator=(Tim &&) = delete;

    void setCounterValue(const uint32_t) const;
    void setAutoReloadValue(const uint32_t) const;
    uint32_t getCounterValue(void) const;
    void enable(void) const;
    void disable(void) const;

    static const uint32_t HALFBRIDGE_PERIODE = 4000;
    static const uint32_t HALL_SENSOR_PRESCALER = 360;
    static const uint32_t BUZZER_PWM_PERIODE = 1000;

    const enum Description mDescription;

private:
    constexpr Tim(const enum Description&        desc,
                  const uint32_t&                peripherie,
                  const TIM_TimeBaseInitTypeDef& conf) :
        mDescription(desc), mPeripherie(peripherie),
        mConfiguration(conf) {}

    const uint32_t mPeripherie;
    const TIM_TimeBaseInitTypeDef mConfiguration;

    void initialize(void) const;
    TIM_TypeDef* getBasePointer(void) const;

    friend class Factory<Tim>;
    friend struct Pwm;
    friend struct HallDecoder;
    friend struct HalfBridge;
    friend struct dev::SensorBLDC;
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
        for (const auto& tim : Container) {
            if (tim.mDescription != Tim::__ENUM__SIZE) {
                tim.initialize();
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
