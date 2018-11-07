// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include <cmath>
#include "stm32f30x_misc.h"
#include "trace.h"
#include "LockGuard.h"
#include "Adc.h"
#include "AdcChannel.h"
#include "os_Task.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Adc;
using hal::Factory;

extern "C" void ADC1_2_IRQHandler(void)
{
    constexpr auto& adc1 = Factory<Adc>::get<Adc::Description::PMD_ADC1>();

    if (ADC_GetITStatus(adc1.getBasePointer(), ADC_FLAG_EOC) == SET) {
        Adc::ConversionCompleteSemaphores[static_cast<size_t>(adc1.mDescription)].giveFromISR();
        ADC_ClearITPendingBit(adc1.getBasePointer(), ADC_FLAG_EOC);
    }

    constexpr auto& adc2 = Factory<Adc>::get<Adc::Description::PMD_ADC2>();

    if (ADC_GetITStatus(adc2.getBasePointer(), ADC_FLAG_EOC) == SET) {
        Adc::ConversionCompleteSemaphores[static_cast<size_t>(adc2.mDescription)].giveFromISR();
        ADC_ClearITPendingBit(adc2.getBasePointer(), ADC_FLAG_EOC);
    }
}

extern "C" void ADC3_IRQHandler(void)
{
    constexpr auto& adc = Factory<Adc>::get<Adc::Description::PMD_ADC3>();

    if (ADC_GetITStatus(adc.getBasePointer(), ADC_FLAG_EOC) == SET) {
        Adc::ConversionCompleteSemaphores[static_cast<size_t>(adc.mDescription)].giveFromISR();
        ADC_ClearITPendingBit(adc.getBasePointer(), ADC_FLAG_EOC);
    }
}

extern "C" void ADC4_IRQHandler(void)
{
    constexpr auto& adc = Factory<Adc>::get<Adc::Description::PMD_ADC4>();

    if (ADC_GetITStatus(adc.getBasePointer(), ADC_FLAG_EOC) == SET) {
        Adc::ConversionCompleteSemaphores[static_cast<size_t>(adc.mDescription)].giveFromISR();
        ADC_ClearITPendingBit(adc.getBasePointer(), ADC_FLAG_EOC);
    }
}

ADC_TypeDef* Adc::getBasePointer(void) const
{
    return reinterpret_cast<ADC_TypeDef*>(mPeripherie);
}

void Adc::initialize(void) const
{
    auto ADCx = this->getBasePointer();

    ADC_CommonInit(ADCx, &mCommonConfiguration);
    ADC_Init(ADCx, &mConfiguration);

    if (mDescription == PMD_ADC1) {
        ADC_TempSensorCmd(ADCx, ENABLE);
    }

    ADC_ITConfig(ADCx, ADC_IT_EOC, ENABLE);

    ADC_Cmd(ADCx, ENABLE);

    while (ADC_GetFlagStatus(ADCx, ADC_FLAG_RDY) == RESET) {
        os::ThisTask::sleep(std::chrono::milliseconds(1));
    }

    ADC_ClearITPendingBit(ADCx, ADC_IT_RDY | ADC_IT_EOC);

    NVIC_SetPriority(mIRQn, Adc::INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(mIRQn);
}

void Adc::calibrate(void) const
{
    auto ADCx = this->getBasePointer();
    ADC_VoltageRegulatorCmd(ADCx, ENABLE);

    /* If ADC does not work proper, wait here for some milliseconds */
    os::ThisTask::sleep(std::chrono::milliseconds(10));

    ADC_SelectCalibrationMode(ADCx, ADC_CalibrationMode_Single);
    ADC_StartCalibration(ADCx);

    while (ADC_GetCalibrationStatus(ADCx) != RESET) {
        os::ThisTask::sleep(std::chrono::milliseconds(1));
    }

    CalibrationValues[static_cast<size_t>(mDescription)] = ADC_GetCalibrationValue(ADCx);
}

uint32_t Adc::getValue(const Adc::Channel& channel) const
{
    os::LockGuard<os::Mutex> lock(ConverterAvailableMutex[static_cast<size_t>(mDescription)]);

    auto ADCx = this->getBasePointer();

    ADC_RegularChannelConfig(ADCx, channel.mChannel, channel.mRank, channel.mSampleTime);

    ADC_StartConversion(ADCx);
    ConversionCompleteSemaphores[static_cast<size_t>(mDescription)].take();

    uint32_t returnValue = ADC_GetConversionValue(ADCx);

    return returnValue;
}

float Adc::getVoltage(const Adc::Channel& channel) const
{
    return (channel.mMaxVoltage / std::pow(2, mResolutionBits)) * getValue(channel);
}

uint32_t Adc::getCalibrationValue(void) const
{
    return CalibrationValues[static_cast<size_t>(mDescription)];
}

void Adc::startConversion(const Adc::Channel& channel) const
{
    auto ADCx = this->getBasePointer();

    ADC_RegularChannelConfig(ADCx, channel.mChannel, channel.mRank, channel.mSampleTime);

    ADC_StartConversion(ADCx);
}

void Adc::stopConversion(void) const
{
    auto ADCx = this->getBasePointer();

    ADC_StopConversion(ADCx);
}

std::array<uint32_t, Adc::Description::__ENUM__SIZE> Adc::CalibrationValues;
std::array<os::Semaphore, Adc::Description::__ENUM__SIZE> Adc::ConversionCompleteSemaphores;
std::array<os::Mutex, Adc::Description::__ENUM__SIZE> Adc::ConverterAvailableMutex;
constexpr std::array<const Adc, Adc::Description::__ENUM__SIZE> Factory<Adc>::Container;
