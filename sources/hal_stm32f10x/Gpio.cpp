// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "Gpio.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Factory;
using hal::Gpio;

Gpio::operator bool() const
{
    auto peripherie = reinterpret_cast<GPIO_TypeDef* const>(mPeripherie);

    if ((mConfiguration.GPIO_Mode == GPIO_Mode_Out_OD) || (mConfiguration.GPIO_Mode == GPIO_Mode_Out_PP)) {
        return GPIO_ReadOutputDataBit(peripherie, (uint16_t)mConfiguration.GPIO_Pin);
    } else {
        return GPIO_ReadInputDataBit(peripherie, (uint16_t)mConfiguration.GPIO_Pin);
    }
}

void Gpio::operator=(const bool& state) const
{
    if ((mConfiguration.GPIO_Mode == GPIO_Mode_Out_OD) || (mConfiguration.GPIO_Mode == GPIO_Mode_Out_PP) ||
        (mConfiguration.GPIO_Mode == GPIO_Mode_AF_PP))
    {
        auto peripherie = reinterpret_cast<GPIO_TypeDef* const>(mPeripherie);
        GPIO_WriteBit(peripherie, static_cast<uint16_t>(mConfiguration.GPIO_Pin), static_cast<BitAction>(state));
    } else {
        Trace(ZONE_WARNING, "GPIO in wrong mode. You can only assign values to output gpios!");
    }
}

void Gpio::configureAsOutput(void) const
{
    if (mConfiguration.GPIO_Mode == GPIO_Mode_AF_PP) {
        GPIO_InitTypeDef configuration = mConfiguration;
        configuration.GPIO_Mode = GPIO_Mode_Out_OD;
        configuration.GPIO_Speed = GPIO_Speed_2MHz;

        GPIO_Init(reinterpret_cast<GPIO_TypeDef* const>(mPeripherie), &configuration);
    }
}
void Gpio::restoreDefaultConfiguration(void) const
{
    initialize();
}

void Gpio::initialize(void) const
{
    GPIO_Init(reinterpret_cast<GPIO_TypeDef* const>(mPeripherie), &mConfiguration);

    if (((mConfiguration.GPIO_Mode == GPIO_Mode_AF_OD) || (mConfiguration.GPIO_Mode == GPIO_Mode_AF_PP))
        && IS_GPIO_PIN_SOURCE(mPinSource))
    {
        //MAYBE Remap here
    }
}

constexpr const std::array<const Gpio, Gpio::Description::__ENUM__SIZE + 1> Factory<Gpio>::Container;
constexpr const std::array<const uint32_t, 7> Factory<Gpio>::Clocks;
constexpr const std::array<const uint32_t, 1> Factory<Gpio>::RemappingContainer;
