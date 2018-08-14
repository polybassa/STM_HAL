/* Copyright (C) 2015  Nils Weiss, Markus Wildgruber
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

#ifndef SOURCES_PMD_EXTI_H_
#define SOURCES_PMD_EXTI_H_

#include <cstdint>
#include <limits>
#include <array>
#include <functional>
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_misc.h"
#include "stm32f4xx.h"
#include "Gpio.h"
#include "hal_Factory.h"

extern "C" {
void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);
void EXTI2_TSC_IRQHandler(void);
void EXTI3_IRQHandler(void);
void EXTI4_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
}

namespace hal
{
struct Exti {
#include "Exti_config.h"

    Exti() = delete;
    Exti(const Exti&) = delete;
    Exti(Exti &&) = default;
    Exti& operator=(const Exti&) = delete;
    Exti& operator=(Exti &&) = delete;

    void enable(void) const;
    void disable(void) const;
    void registerInterruptCallback(std::function<void(void)> ) const;
    void unregisterInterruptCallback(void) const;
    void handleInterrupt(void) const;

private:
    constexpr Exti(const enum Description&   desc,
                   const Gpio&               gpio,
                   const EXTITrigger_TypeDef trigger,
                   const uint32_t            priority = 0xf) :
        mDescription(desc), mGpio(gpio),
        mConfiguration(EXTI_InitTypeDef
                       {
                           gpio
                           .mPinSource, EXTI_Mode_Interrupt,
                           trigger,
                           DISABLE
                       }),
        mPriority(priority) {}

    const enum Description mDescription;
    const Gpio& mGpio;
    const EXTI_InitTypeDef mConfiguration;
    const uint32_t mPriority;

    uint8_t getExtiPortSource(const uint32_t& peripherieBase) const;
    IRQn getIRQChannel(const EXTI_InitTypeDef& config) const;
    void setEXTI_LineCmd(FunctionalState cmd) const;
    void clearInterruptBit(void) const;
    void executeCallback(void) const;
    void initialize(void) const;
    bool getStatus(void) const;

    using CallbackArray = std::array<std::function<void(void)>, Exti::__ENUM__SIZE>;
    static CallbackArray ExtiCallbacks;

    friend class Factory<Exti>;
};

template<>
class Factory<Exti>
{
#include "Exti_config.h"

    Factory(void)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

        RCC_APB2PeriphResetCmd(RCC_APB2Periph_SYSCFG, ENABLE);
        RCC_APB2PeriphResetCmd(RCC_APB2Periph_SYSCFG, DISABLE);

        for (const auto& obj : Container) {
            obj.initialize();
        }
    }

    template<uint32_t Exti_Line, enum Exti::Description index>
    static constexpr const Exti& getByExtiLine(void)
    {
        return (Container[index]).mGpio.mPinSource ==
               Exti_Line ? Container[index] : getByExtiLine<Exti_Line,
                                                            static_cast<enum Exti::Description>(index - 1)>();
    }

public:

    template<enum Exti::Description index>
    static constexpr const Exti& get(void)
    {
        static_assert(IS_EXTI_MODE(Container[index].mConfiguration.EXTI_Mode), "Invalid Mode");
        static_assert(IS_EXTI_PIN_SOURCE(Container[index].mGpio.mPinSource), "Invalid PinSource");
        static_assert(IS_EXTI_LINE_ALL(Container[index].mConfiguration.EXTI_Line), "Invalid Line");
        static_assert(IS_EXTI_TRIGGER(Container[index].mConfiguration.EXTI_Trigger), "Invalid Trigger");
        static_assert(Container[index].mConfiguration.EXTI_Line == Container[index].mGpio.mPinSource,
                      "Exti declaration in the wrong place. EXTI_LINE must be equal gpio pin Source");

        static_assert(
                      Container[index].mDescription ==
                      getByExtiLine<Container[index].mConfiguration.EXTI_Line>().mDescription,
                      "Can not access Exti correct");

        static_assert(index != Exti::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<uint32_t Exti_Line>
    static constexpr const Exti& getByExtiLine(void)
    {
        static_assert(IS_EXTI_LINE_EXT(Exti_Line), "Invalid Line ");
        return getByExtiLine<Exti_Line,
                             static_cast<enum Exti::Description>(Exti::Description::__ENUM__SIZE - 1)>();
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_EXTI_H_ */
