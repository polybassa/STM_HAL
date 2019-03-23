// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_EXTI_H_
#define SOURCES_PMD_EXTI_H_

#include <cstdint>
#include <limits>
#include <array>
#include <functional>
#include "stm32f10x_exti.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x.h"
#include "Gpio.h"
#include "hal_Factory.h"

extern "C" {
void    EXTI0_IRQHandler(void);
void    EXTI1_IRQHandler(void);
void    EXTI2_IRQHandler(void);
void    EXTI3_IRQHandler(void);
void    EXTI4_IRQHandler(void);
void    EXTI9_5_IRQHandler(void);
void    EXTI15_10_IRQHandler(void);
}

namespace hal
{
struct Exti {
#include "Exti_config.h"

    Exti() = delete;
    Exti(const Exti&) = delete;
    Exti(Exti&&) = default;
    Exti& operator=(const Exti&) = delete;
    Exti& operator=(Exti&&) = delete;

    void enable(void) const;
    void disable(void) const;
    void registerInterruptCallback(std::function<void(void)> ) const;
    void unregisterInterruptCallback(void) const;
    void handleInterrupt(void) const;

private:
    constexpr Exti(const enum Description& desc,
                   const EXTI_InitTypeDef  configuration,
                   const uint8_t           GPIO_PortSource,
                   const uint8_t           GPIO_PinSource) :
        mDescription(desc),
        mConfiguration(configuration),
        mPortSource(GPIO_PortSource),
        mPinSource(GPIO_PinSource) {}

    const enum Description mDescription;
    const EXTI_InitTypeDef mConfiguration;
    const uint8_t mPortSource;
    const uint8_t mPinSource;

    uint8_t getExtiPortSource(const uint32_t& peripherieBase) const;
    IRQn getIRQChannel(const EXTI_InitTypeDef& config) const;
    void setEXTI_LineCmd(FunctionalState cmd) const;
    void clearInterruptBit(void) const;
    void executeCallback(void) const;
    void initialize(void) const;
    bool getStatus(void) const;

    using CallbackArray = std::array<std::function<void (void)>, Exti::__ENUM__SIZE>;
    static CallbackArray ExtiCallbacks;

    friend class Factory<Exti>;
};

template<>
class Factory<Exti>
{
#include "Exti_config.h"

    Factory(void)
    {
        for (const auto& obj : Container) {
            obj.initialize();
        }
    }

    template<uint32_t Exti_Line, enum Exti::Description index>
    static constexpr const Exti& getByExtiLine(void)
    {
        return (Container[index]).mConfiguration.EXTI_Line ==
               Exti_Line ? Container[index] : getByExtiLine<Exti_Line,
                                                            static_cast<enum Exti::Description>(index - 1)>();
    }

public:

    template<enum Exti::Description index>
    static constexpr const Exti& get(void)
    {
        static_assert(IS_EXTI_MODE(Container[index].mConfiguration.EXTI_Mode), "Invalid Mode");
        static_assert(IS_EXTI_LINE(Container[index].mConfiguration.EXTI_Line), "Invalid Line");
        static_assert(IS_EXTI_TRIGGER(Container[index].mConfiguration.EXTI_Trigger), "Invalid Trigger");
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
        static_assert(IS_EXTI_LINE(Exti_Line), "Invalid Line ");
        return getByExtiLine<Exti_Line,
                             static_cast<enum Exti::Description>(Exti::Description::__ENUM__SIZE - 1)>();
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_EXTI_H_ */
