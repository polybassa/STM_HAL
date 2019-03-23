// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "TaskInterruptable.h"
#include "Exti.h"
#include "Adc.h"
#include "Dma.h"
#include "Usb.h"
#include "Tim.h"
#include "HorrorscopeLib.h"

namespace app
{
class Horrorscope final
{
    static constexpr size_t STACKSIZE = 1024;

    os::TaskInterruptable mTask;
    const hal::Exti& mTrigger;
    const hal::Adc& mAdc1;
    const hal::Adc& mAdc2;
    const hal::Dma& mDma;
    const hal::Usb& mUsb;
    const hal::Tim& mTim;
    const hal::Gpio& mLed;

    static char getChar(void);
    static uint16_t getShort(void);
    static void putChar(const char);
    static void putBuffer(uint8_t const* const, const uint16_t);
    static void putString(const char*);
    static uint8_t triggerWaitFunction(const uint32_t, const uint16_t);

    void taskFunction(const bool&);

    static HorrorscopeData mData;
    HorrorscopeFunctions mFunctions;
    bool mCaptureDone = false;

public:
    Horrorscope(const hal::Exti& trigger,
                const hal::Adc&  adc1,
                const hal::Adc&  adc2,
                const hal::Dma&  dma,
                const hal::Usb&  usb,
                const hal::Tim&  tim,
                const hal::Gpio& led);

    Horrorscope(const Horrorscope&) = delete;
    Horrorscope(Horrorscope&&) = delete;
    Horrorscope& operator=(const Horrorscope&) = delete;
    Horrorscope& operator=(Horrorscope&&) = delete;
};
}
