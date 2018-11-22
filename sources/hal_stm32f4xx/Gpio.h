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

#ifndef SOURCES_PMD_GPIO_H_
#define SOURCES_PMD_GPIO_H_

#include <cstdint>
#include <limits>
#include <array>
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "hal_Factory.h"

// ================================================================================================
// Added preprocessor macros that are not part of the stm32f4 std periph lib,
// but of those for the other stm32fx controllers.
#define IS_GPIO_ALL_PERIPH_BASE(PERIPH) (((PERIPH) == GPIOA_BASE) || \
                                         ((PERIPH) == GPIOB_BASE) || \
                                         ((PERIPH) == GPIOC_BASE) || \
                                         ((PERIPH) == GPIOD_BASE) || \
                                         ((PERIPH) == GPIOE_BASE) || \
                                         ((PERIPH) == GPIOF_BASE) || \
                                         ((PERIPH) == GPIOG_BASE) || \
                                         ((PERIPH) == GPIOH_BASE) || \
                                         ((PERIPH) == GPIOI_BASE) || \
                                         ((PERIPH) == GPIOJ_BASE) || \
                                         ((PERIPH) == GPIOK_BASE))
// ================================================================================================

namespace hal
{
struct Exti;

struct Gpio {
#include "Gpio_config.h"

    Gpio() = delete;
    Gpio(const Gpio&) = delete;
    Gpio(Gpio &&) = default;
    Gpio& operator=(const Gpio&) = delete;
    Gpio& operator=(Gpio &&) = delete;

    explicit operator bool() const;

    void operator=(const bool& state) const;

private:
    constexpr Gpio(const Description&        desc,
                   const uint32_t&           peripherie,
                   const GPIO_InitTypeDef && conf,
                   const uint16_t&           pinSource = std::numeric_limits<uint16_t>::max(),
                   const uint8_t&            AF = std::numeric_limits<uint8_t>::max()) :
        mDescription(desc),
        mPeripherie(peripherie),
        mConfiguration(std::move(conf)),
        mPinSource(pinSource),
        mAF(AF) {}

    const Description mDescription;
    const uint32_t mPeripherie;
    const GPIO_InitTypeDef mConfiguration;
    const uint16_t mPinSource;
    const uint8_t mAF;

    void initialize(void) const;

    template<typename>
    friend class Factory;
    friend struct Exti;
};

template<>
class Factory<Gpio>
{
#include "Gpio_config.h"

    static constexpr const std::array<const uint32_t, 11> Clocks =
    { {
          RCC_AHB1ENR_GPIOAEN,
          RCC_AHB1ENR_GPIOBEN,
          RCC_AHB1ENR_GPIOCEN,
          RCC_AHB1ENR_GPIODEN,
          RCC_AHB1ENR_GPIOEEN,
          RCC_AHB1ENR_GPIOFEN,
          RCC_AHB1ENR_GPIOGEN,
          RCC_AHB1ENR_GPIOHEN,
          RCC_AHB1ENR_GPIOIEN,
          RCC_AHB1ENR_GPIOJEN,
          RCC_AHB1ENR_GPIOKEN
      } };

    Factory(void)
    {
        for (const auto& clock : Clocks) {
            RCC_AHB1PeriphClockCmd(clock, ENABLE);

            // Alternative way of GPIO_DeInit(X)
            RCC_AHB1PeriphResetCmd(clock, ENABLE);
            RCC_AHB1PeriphResetCmd(clock, DISABLE);
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
        static_assert(IS_GPIO_OTYPE(Container[index].mConfiguration.GPIO_OType), "Invalid OType");
        static_assert(IS_GPIO_PUPD(Container[index].mConfiguration.GPIO_PuPd), "Invalid PuPd value ");
        static_assert((Container[index].mConfiguration.GPIO_Mode != GPIO_Mode_AF) ||
                      IS_GPIO_PIN_SOURCE(Container[index].mPinSource), "Invalid Pin_Source");
        static_assert((Container[index].mConfiguration.GPIO_Mode != GPIO_Mode_AF) || IS_GPIO_AF(
                                                                                                Container[index].mAF),
                      "Invalid AF");
        static_assert(index != Gpio::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(index < Container[index + 1].mDescription, "Incorrect order of Gpios in GpioFactory");
        static_assert(Container[index].mPeripherie <= Container[index + 1].mPeripherie,
                      "Incorrect order of Gpios in GpioFactory");
        static_assert(
                      Container[index].mPeripherie < Container[index + 1].mPeripherie ||
                      Container[index].mConfiguration.GPIO_Pin < Container[index + 1].mConfiguration.GPIO_Pin,
                      "Incorrect order of Gpios in GpioFactory");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

public:

    template<enum Gpio::Description index>
    static constexpr const Gpio& get(void)
    {
        static_assert(Container[index].mConfiguration.GPIO_Mode != GPIO_Mode_AN,
                      "You can not access analog pins for an GPIO object");
        static_assert(Container[index].mConfiguration.GPIO_Mode != GPIO_Mode_AF,
                      "You can not access alternateFunction pins for a GPIO object");
        return getGpio<index>();
    }

    template<enum Gpio::Description index>
    static constexpr const Gpio& getAlternateFunctionGpio(void)
    {
        static_assert(Container[index].mConfiguration.GPIO_Mode == GPIO_Mode_AF,
                      "You can only access alternateFunction pin for a GPIO object");
        return getGpio<index>();
    }

    template<typename U>
    friend const U& getFactory(void);

    friend class Factory<Exti>;
};
}

#endif /* SOURCES_PMD_GPIO_H_ */
