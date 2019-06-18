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
#include "AdcWithDma.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR |
                                                        ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;
using hal::Adc;
using hal::AdcWithDma;
using hal::Dma;
using hal::Factory;

void AdcWithDma::initialize(void) const
{
    if (!IS_ADC_DMA_ACCESS_MODE(mAdcDmaMode)) {
        return;
    }

    mAdcChannel.mBaseAdc.mNvic.disable();

    ADC_DMACmd(mAdcChannel.mBaseAdc.getBasePointer(), ENABLE);
}

void AdcWithDma::startConversion(uint16_t const* const data, const size_t length,
                                 os::Semaphore* dataAvailableSemaphore) const
{
    mDma.setupTransfer(reinterpret_cast<uint8_t const* const>(data), length, true);
    if (dataAvailableSemaphore != nullptr) {
        mDma.registerInterruptSemaphore(dataAvailableSemaphore, Dma::TC);
    }

    mDma.enable();
    mAdcChannel.startConversion();
}

void AdcWithDma::startConversion(uint16_t const* const data, const size_t length,
                                 std::function<void(void)> callBack) const
{
    mDma.setupTransfer(reinterpret_cast<uint8_t const* const>(data), length, true);
    mDma.registerInterruptCallback(callBack, Dma::TC);

    mDma.enable();
    mAdcChannel.startConversion();
}

void AdcWithDma::stopConversion(void) const
{
//    mAdcChannel.stopConversion(); // not available on the stm32f4
    mDma.disable();
}

float AdcWithDma::getVoltage(const uint16_t value) const
{
    return mAdcChannel.getVoltage(value);
}

float AdcWithDma::getVoltage(const float value) const
{
    return mAdcChannel.getVoltage(value);
}

uint32_t AdcWithDma::getAdcSampleTime(void) const
{
    return mAdcChannel.mSampleTime;
}

constexpr const std::array<const AdcWithDma,
                           Factory<AdcWithDma>::NUMBER_OF_ADC_WITH_DMA> Factory<AdcWithDma>::Container;
