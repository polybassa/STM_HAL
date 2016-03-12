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

#include <cmath>
#include "trace.h"
#include "Dac.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Dac;
using hal::Factory;

void Dac::initialize(void) const
{
    DAC_Init(reinterpret_cast<DAC_TypeDef*>(mPeripherie), mChannel, &mConfiguration);

    DAC_Cmd(reinterpret_cast<DAC_TypeDef*>(mPeripherie), mChannel, ENABLE);

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

void Dac::set(const uint32_t data) const
{
    set(data, mAlign);
}

void Dac::set(const uint32_t data, const uint16_t align) const
{
    if (mConfiguration.DAC_Trigger == DAC_Trigger_Software) {
        DAC_SoftwareTriggerCmd(reinterpret_cast<DAC_TypeDef*>(mPeripherie), mChannel, ENABLE);
    }

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
    DAC_SoftwareTriggerCmd(reinterpret_cast<DAC_TypeDef*>(mPeripherie), mChannel, )
}

constexpr std::array<const Dac, Dac::Description::__ENUM__SIZE> Factory<Dac>::Container;
