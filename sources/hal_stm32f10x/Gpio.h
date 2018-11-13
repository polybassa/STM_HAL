// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_GPIO_H_
#define SOURCES_PMD_GPIO_H_

#include <cstdint>
#include <limits>
#include <array>
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "hal_Factory.h"

namespace hal
{
struct Exti;

struct Gpio {
#include "Gpio_config.h"

    Gpio() = delete;
    Gpio(const Gpio&) = delete;
    Gpio(Gpio&&) = default;
    Gpio& operator=(const Gpio&) = delete;
    Gpio& operator=(Gpio&&) = delete;

    operator bool() const;
    void operator=(const bool& state) const;

    void configureAsOutput(void) const;
    void restoreDefaultConfiguration(void) const;

private:
    constexpr Gpio(const Description&       desc,
                   const uint32_t&          peripherie,
                   const GPIO_InitTypeDef&& conf,
                   const uint16_t&          pinSource = std::numeric_limits<uint16_t>::max()) :
        mDescription(desc),
        mPeripherie(peripherie),
        mConfiguration(std::move(conf)),
        mPinSource(pinSource) {}

    const Description mDescription;
    const uint32_t mPeripherie;
    const GPIO_InitTypeDef mConfiguration;
    const uint16_t mPinSource;

    void initialize(void) const;

    template<typename>
    friend class Factory;
    friend struct Exti;
};

template<>
class Factory<Gpio>
{
#include "Gpio_config.h"

    static constexpr const std::array<const uint32_t, 7> Clocks =
    { {
          RCC_APB2Periph_GPIOA,
          RCC_APB2Periph_GPIOB,
          RCC_APB2Periph_GPIOC,
          RCC_APB2Periph_GPIOD,
          RCC_APB2Periph_GPIOE,
          RCC_APB2Periph_GPIOF,
          RCC_APB2Periph_GPIOG
      } };

    Factory(void)
    {
        for (const auto& clock : Clocks) {
            RCC_APB2PeriphClockCmd(clock, ENABLE);

            RCC_APB2PeriphResetCmd(clock, ENABLE);
            RCC_APB2PeriphResetCmd(clock, DISABLE);
        }

        for (const auto& gpioremap : RemappingContainer) {
            GPIO_PinRemapConfig(gpioremap, ENABLE);
        }

        for (const auto& gpio : Container) {
            if (gpio.mDescription != Gpio::__ENUM__SIZE) {
                gpio.initialize();
            }
        }
    }
    template<enum Gpio::Description index>
    static constexpr const Gpio& getGpio(void)
    {
        static_assert(IS_GPIO_ALL_PERIPH_BASE(Container[index].mPeripherie), "Invalid Peripheries ");
        static_assert(IS_GPIO_SPEED(Container[index].mConfiguration.GPIO_Speed), "Invalid Speed");
        static_assert((Container[index].mConfiguration.GPIO_Mode != GPIO_Mode_AF_OD) ||
                      IS_GPIO_PIN_SOURCE(Container[index].mPinSource), "Invalid Pin_Source");
        static_assert((Container[index].mConfiguration.GPIO_Mode != GPIO_Mode_AF_PP) ||
                      IS_GPIO_PIN_SOURCE(Container[index].mPinSource), "Invalid Pin_Source");
        static_assert(index != Gpio::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(index < Container[index + 1].mDescription, "Incorrect order of Gpios in GpioFactory");
        static_assert(Container[index].mPeripherie <= Container[index + 1].mPeripherie,
                      "Incorrect order of Gpios in GpioFactory");
        static_assert(Container[index].mPeripherie < Container[index + 1].mPeripherie ||
                      Container[index].mConfiguration.GPIO_Pin < Container[index + 1].mConfiguration.GPIO_Pin,
                      "Incorrect order of Gpios in GpioFactory");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

public:

    template<enum Gpio::Description index>
    static constexpr const Gpio& get(void)
    {
        static_assert(Container[index].mConfiguration.GPIO_Mode != GPIO_Mode_AIN,
                      "You can not access analog pins for an GPIO object");
        static_assert(Container[index].mConfiguration.GPIO_Mode != GPIO_Mode_AF_OD,
                      "You can not access alternateFunction pins for a GPIO object");
        static_assert(Container[index].mConfiguration.GPIO_Mode != GPIO_Mode_AF_PP,
                      "You can not access alternateFunction pins for a GPIO object");
        return getGpio<index>();
    }

    template<enum Gpio::Description index>
    static constexpr const Gpio& getAlternateFunctionGpio(void)
    {
        static_assert(
                      Container[index].mConfiguration.GPIO_Mode == GPIO_Mode_AF_OD ||
                      Container[index].mConfiguration.GPIO_Mode ==
                      GPIO_Mode_AF_PP,
                      "You can only access alternateFunction pin for a GPIO object");
        return getGpio<index>();
    }

    template<typename U>
    friend const U& getFactory(void);

    friend class Factory<Exti>;
};
}

#endif /* SOURCES_PMD_GPIO_H_ */
