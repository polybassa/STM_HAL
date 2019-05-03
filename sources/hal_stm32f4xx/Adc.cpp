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

#include "stm32f4xx_misc.h"
#include "trace.h"
#include "LockGuard.h"
#include "Adc.h"
#include "AdcChannel.h"
#include "os_Task.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Adc;
using hal::Factory;

ADC_TypeDef* Adc::getBasePointer(void) const
{
    return reinterpret_cast<ADC_TypeDef*>(mPeripherie);
}

void Adc::initialize(void) const
{
    auto ADCx = this->getBasePointer();

    ADC_CommonInitTypeDef tmpCommonConfig = mCommonConfiguration;
    ADC_CommonInit(&tmpCommonConfig);

    ADC_InitTypeDef tmpConfig = mConfiguration;
    ADC_Init(ADCx, &tmpConfig);

    ADC_ITConfig(ADCx, ADC_IT_EOC, ENABLE);

    ADC_Cmd(ADCx, ENABLE);

    // ready flag was removed for the STM32F4

    ADC_ClearITPendingBit(ADCx, ADC_IT_EOC);

    mNvic.setPriority(static_cast<const uint32_t>(Adc::INTERRUPT_PRIORITY));

    // Interrupt status and cleanup for all available adcs, because all share one IRQ
    mNvic.registerGetInterruptStatusProcedure([](void){
        return (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == SET) ||
        (ADC_GetFlagStatus(ADC2, ADC_FLAG_EOC) == SET) ||
        (ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC) == SET);
    });

    mNvic.registerClearInterruptProcedure([](void){
        ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
        ADC_ClearFlag(ADC2, ADC_FLAG_EOC);
        ADC_ClearFlag(ADC3, ADC_FLAG_EOC);
    });

    mNvic.registerInterruptCallback([](void){
        hal::Adc::handleInterrupt();
    });

    // Nvic can always be enabled, because it just checks whether the interrupt procedures were set.
    mNvic.enable();
}

uint16_t Adc::getValue(const Adc::Channel& channel) const
{
    os::LockGuard<os::Mutex> lock(ConverterAvailableMutex[static_cast<size_t>(mDescription)]);

    auto ADCx = this->getBasePointer();

    ADC_RegularChannelConfig(ADCx, channel.mChannel, channel.mRank, channel.mSampleTime);

    ADC_SoftwareStartConv(ADCx);
    if (ConversionCompleteSemaphores[static_cast<size_t>(mDescription)]) {
        ConversionCompleteSemaphores[static_cast<size_t>(mDescription)].take();
    }

    uint16_t returnValue = ADC_GetConversionValue(ADCx);

    return returnValue;
}

float Adc::getVoltage(const Adc::Channel& channel) const
{
    return channel.mMaxVoltage / mResolution * getValue(channel);
}

void Adc::startConversion(const Adc::Channel& channel) const
{
    auto ADCx = this->getBasePointer();

    ADC_RegularChannelConfig(ADCx, channel.mChannel, channel.mRank, channel.mSampleTime);

    ADC_SoftwareStartConv(ADCx);
}

void Adc::handleInterrupt(void)
{
    constexpr auto& adc1 = Factory<Adc>::get<Adc::Description::HAL_ADC1>();
    constexpr auto& adc2 = Factory<Adc>::get<Adc::Description::HAL_ADC2>();
    constexpr auto& adc3 = Factory<Adc>::get<Adc::Description::HAL_ADC3>();

    if (ADC_GetFlagStatus(adc1.getBasePointer(), ADC_FLAG_EOC) == SET) {
        Adc::ConversionCompleteSemaphores[static_cast<size_t>(adc1.mDescription)].giveFromISR();
    } else if (ADC_GetFlagStatus(adc2.getBasePointer(), ADC_FLAG_EOC) == SET) {
        Adc::ConversionCompleteSemaphores[static_cast<size_t>(adc2.mDescription)].giveFromISR();
    } else if (ADC_GetFlagStatus(adc3.getBasePointer(), ADC_FLAG_EOC) == SET) {
        Adc::ConversionCompleteSemaphores[static_cast<size_t>(adc3.mDescription)].giveFromISR();
    }
}

std::array<os::Semaphore, Adc::Description::__ENUM__SIZE> Adc::ConversionCompleteSemaphores;
std::array<os::Mutex, Adc::Description::__ENUM__SIZE> Adc::ConverterAvailableMutex;
constexpr std::array<const Adc, Adc::Description::__ENUM__SIZE> Factory<Adc>::Container;
