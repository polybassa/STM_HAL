// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "trace.h"
#include "Adc.h"
#include "os_Task.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Adc;
using hal::AdcChannel;
using hal::Factory;

void Adc::initialize(void) const
{
    auto ADCx = reinterpret_cast<ADC_TypeDef*>(mPeripherie);

    ADC_Init(ADCx, &mConfiguration);
    if (mDmaSupport) {
        ADC_DMACmd(ADCx, ENABLE);
    }

    ADC_Cmd(ADCx, ENABLE);
}

void Adc::calibrate(void) const
{
    ADC_ResetCalibration(reinterpret_cast<ADC_TypeDef*>(mPeripherie));
    while (ADC_GetResetCalibrationStatus(reinterpret_cast<ADC_TypeDef*>(mPeripherie))) {
        os::ThisTask::sleep(std::chrono::milliseconds(1));
    }

    ADC_StartCalibration(reinterpret_cast<ADC_TypeDef*>(mPeripherie));
    while (ADC_GetCalibrationStatus(reinterpret_cast<ADC_TypeDef*>(mPeripherie))) {
        os::ThisTask::sleep(std::chrono::milliseconds(1));
    }
}

void Adc::startConversion(void) const
{
    ADC_SoftwareStartConvCmd(reinterpret_cast<ADC_TypeDef*>(mPeripherie), ENABLE);
}

constexpr std::array<const Adc, Adc::Description::__ENUM__SIZE> Factory<Adc>::Container;
constexpr std::array<const AdcChannel, 2> Factory<Adc>::ChannelContainer;
