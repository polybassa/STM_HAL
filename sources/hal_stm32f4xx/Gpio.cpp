/* Copyright (C) 2018  Nils Weiss and Henning Mende
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

#include "Gpio.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Factory;
using hal::Gpio;

GPIOMode_TypeDef Gpio::getModeFromHardware(void) const
{
    if (mReconfigurable) {
        auto peripherie = reinterpret_cast<GPIO_TypeDef* const>(mPeripherie);
        return static_cast<GPIOMode_TypeDef>(((peripherie->MODER) & (0x03 << (mPinSource * 2))) >> (mPinSource * 2));
    } else {
        return mConfiguration.GPIO_Mode;
    }
}

Gpio::operator bool() const
{
    auto peripherie = reinterpret_cast<GPIO_TypeDef* const>(mPeripherie);

    if (getModeFromHardware() == GPIO_Mode_OUT) {
        return GPIO_ReadOutputDataBit(peripherie, (uint16_t)mConfiguration.GPIO_Pin);
    } else {
        return GPIO_ReadInputDataBit(peripherie, (uint16_t)mConfiguration.GPIO_Pin);
    }
}

void Gpio::operator=(const bool& state) const
{
    if (getModeFromHardware() == GPIO_Mode_OUT) {
        auto peripherie = reinterpret_cast<GPIO_TypeDef*>(mPeripherie);
        GPIO_WriteBit(peripherie, static_cast<uint16_t>(mConfiguration.GPIO_Pin), static_cast<BitAction>(state));
    } else {
        Trace(ZONE_WARNING, "GPIO in wrong mode. You can only assign values to output gpios!");
    }
}

void Gpio::initialize(void) const
{
    const auto peripherie = reinterpret_cast<GPIO_TypeDef*>(mPeripherie);
    const auto configuration = static_cast<GPIO_InitTypeDef const*>(&mConfiguration);

    GPIO_Init(peripherie, configuration);

    if ((mConfiguration.GPIO_Mode == GPIO_Mode_AF) && IS_GPIO_PIN_SOURCE(mPinSource) && IS_GPIO_AF(mAF)) {
        GPIO_PinAFConfig(peripherie, mPinSource, mAF);
    }
}

void Gpio::changeAlternateFunction(const uint8_t afMode) const
{
    if (!mReconfigurable) {
        Trace(ZONE_WARNING, "Pin must be reconfigurable, to change the alternate function!\n");
        return;
    }

    if (IS_GPIO_AF(afMode)) {
        const auto peripherie = reinterpret_cast<GPIO_TypeDef*>(mPeripherie);

        GPIO_InitTypeDef configuration = mConfiguration;
        configuration.GPIO_Mode = GPIO_Mode_AF;

        GPIO_PinAFConfig(peripherie, mPinSource, afMode);
        GPIO_Init(peripherie, &configuration);
    } else {
        Trace(ZONE_WARNING, "Invalid alternate function!\n");
    }
}

void Gpio::changeGpioMode(const GPIOMode_TypeDef mode) const
{
    if (!mReconfigurable) {
        Trace(ZONE_WARNING, "Pin must be reconfigurable, to change the mode!\n");
        return;
    }

    if (IS_GPIO_MODE(mode)) {
        GPIO_TypeDef* peripherie = reinterpret_cast<GPIO_TypeDef*>(mPeripherie);
        GPIO_InitTypeDef configuration = mConfiguration;
        configuration.GPIO_Mode = mode;
        if (mode == GPIO_Mode_IN) {
            configuration.GPIO_OType = GPIO_OType_OD;
        } else if (mode == GPIO_Mode_OUT) {
            configuration.GPIO_OType = GPIO_OType_PP;
            configuration.GPIO_PuPd = GPIO_PuPd_NOPULL;
        }

        GPIO_Init(peripherie, &configuration);
    }
}

constexpr const std::array<const Gpio, Gpio::Description::__ENUM__SIZE + 1> Factory<Gpio>::Container;
constexpr const std::array<const uint32_t, 11> Factory<Gpio>::Clocks;
