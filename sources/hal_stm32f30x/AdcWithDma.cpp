// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

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
    if (!IS_ADC_DMA_MODE(mAdcDmaMode)) {
        return;
    }

    NVIC_DisableIRQ(mAdcChannel.mBaseAdc.mIRQn);

    ADC_DMAConfig(mAdcChannel.mBaseAdc.getBasePointer(), mAdcDmaMode);
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
    mAdcChannel.stopConversion();
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
