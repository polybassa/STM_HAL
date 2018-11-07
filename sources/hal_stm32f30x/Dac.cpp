// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include <cmath>
#include "trace.h"
#include "Dac.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Dac;
using hal::Factory;

void Dac::initialize(void) const
{
    DAC_Init(reinterpret_cast<DAC_TypeDef*>(mPeripherie), mChannel, const_cast<DAC_InitTypeDef*>(&mConfiguration));

    if (mConfiguration.DAC_WaveGeneration == DAC_WaveGeneration_Noise) {
        set(0x7ff0, DAC_Align_12b_L);

        DAC_WaveGenerationCmd(reinterpret_cast<DAC_TypeDef*>(mPeripherie), mChannel, DAC_Wave_Noise, ENABLE);
    } else if (mConfiguration.DAC_WaveGeneration == DAC_WaveGeneration_Triangle) {
        DAC_WaveGenerationCmd(reinterpret_cast<DAC_TypeDef*>(mPeripherie), mChannel, DAC_Wave_Triangle, ENABLE);
    } else { DAC_WaveGenerationCmd(reinterpret_cast<DAC_TypeDef*>(mPeripherie), mChannel, DAC_Wave_Triangle, DISABLE); }

    if (mDma) {
        DAC_DMACmd(reinterpret_cast<DAC_TypeDef*>(mPeripherie), mChannel, ENABLE);
        mDma->enable();
    }
}

void Dac::enable(void) const
{
    DAC_Cmd(reinterpret_cast<DAC_TypeDef*>(mPeripherie), mChannel, ENABLE);
}

void Dac::disable(void) const
{
    DAC_Cmd(reinterpret_cast<DAC_TypeDef*>(mPeripherie), mChannel, DISABLE);
}

void Dac::set(const uint32_t data) const
{
    set(data, mAlign);
}

void Dac::set(const uint32_t data, const uint16_t align) const
{
    trigger();

    if (mChannel == DAC_Channel_1) {
        DAC_SetChannel1Data(reinterpret_cast<DAC_TypeDef*>(mPeripherie), align, data);
    } else {
        DAC_SetChannel2Data(reinterpret_cast<DAC_TypeDef*>(mPeripherie), align, data);
    }
}

uint16_t Dac::get(void) const
{
    return DAC_GetDataOutputValue(reinterpret_cast<DAC_TypeDef*>(mPeripherie), mChannel);
}

void Dac::trigger(void) const
{
    DAC_SoftwareTriggerCmd(reinterpret_cast<DAC_TypeDef*>(mPeripherie), mChannel, ENABLE);
}

constexpr std::array<const Dac, Dac::Description::__ENUM__SIZE> Factory<Dac>::Container;
