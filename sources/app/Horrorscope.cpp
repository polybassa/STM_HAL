// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "Horrorscope.h"
#include "trace.h"
#include <cstring>

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using app::Horrorscope;

static Horrorscope* scope;
HorrorscopeData Horrorscope::mData;

extern volatile uint32_t Receive_length;

Horrorscope::Horrorscope(const hal::Exti& trigger,
                         const hal::Adc&  adc1,
                         const hal::Adc&  adc2,
                         const hal::Dma&  dma,
                         const hal::Usb&  usb,
                         const hal::Tim&  tim,
                         const hal::Gpio& led) :
    mTask("HorrorTask",
          Horrorscope::STACKSIZE,
          os::Task::Priority::HIGH,
          [&](const bool& join){
    taskFunction(join);
}),
    mTrigger(trigger),
    mAdc1(adc1),
    mAdc2(adc2),
    mDma(dma),
    mUsb(usb), mTim(tim), mLed(led)
{
    scope = this;
}

char Horrorscope::getChar(void)
{
    char ret = 0;
    if (scope->mUsb.isConfigured()) {
        auto len = scope->mUsb.receive((uint8_t*)&ret, sizeof(ret), std::chrono::milliseconds(1000));
        if (len != sizeof(ret)) {
            Trace(ZONE_INFO, "Receive Failed\r\n");
            return 0;
        }
    }
    Trace(ZONE_INFO, "Receive Char %c bytesLeft = %d\r\n", ret, Receive_length);

    return ret;
}

uint16_t Horrorscope::getShort(void)
{
    uint16_t ret = 0;
    if (scope->mUsb.isConfigured()) {
        auto len = scope->mUsb.receive((uint8_t*)&ret, sizeof(ret), std::chrono::milliseconds(100));
        if (len != sizeof(ret)) {
            Trace(ZONE_INFO, "Receive Failed\r\n");
            return 0;
        }
    }
    Trace(ZONE_INFO, "Receive Short 0x%04x bytesLeft = %d\r\n", ret, Receive_length);

    return ret;
}

void Horrorscope::putChar(const char data)
{
    if (scope->mUsb.isConfigured()) {
        scope->mUsb.send((const uint8_t*)&data, sizeof(data));
    }
}

void Horrorscope::putBuffer(uint8_t const* const data, const uint16_t len)
{
    if (scope->mUsb.isConfigured()) {
        Trace(ZONE_INFO, "sending buffer\r\n");

        for (int i = 0; i < len; i = i + 2) {
            SEGGER_RTT_printf(0, "0x%02x%02x ", data[i + 1], data[i]);
        }
        SEGGER_RTT_printf(0, "\r\n");

        scope->mUsb.send(data, len);
    }
}

void Horrorscope::putString(const char* str)
{
    if (scope->mUsb.isConfigured()) {
        scope->mUsb.send((const uint8_t*)str, std::strlen(str));
    }
}

uint8_t Horrorscope::triggerWaitFunction(const uint32_t timeout, const uint16_t delay)
{
    Trace(ZONE_INFO, "Armed\r\n");
    scope->mCaptureDone = false;
    scope->mTrigger.enable();

    for (size_t i = 0; (i < timeout) && scope->mCaptureDone == false; i++) {
        os::ThisTask::sleep(std::chrono::milliseconds(1));
    }
    Trace(ZONE_INFO, scope->mCaptureDone ? "captured\r\n" : "timeout\r\n");
    return scope->mCaptureDone ? 0 : 1;
}

void Horrorscope::taskFunction(const bool& t)
{
    mFunctions.init = [] {};
    mFunctions.usb_get_char = Horrorscope::getChar;
    mFunctions.usb_get_short = Horrorscope::getShort;
    mFunctions.usb_put_char = Horrorscope::putChar;
    mFunctions.usb_put_buffer = Horrorscope::putBuffer;
    mFunctions.usb_put_string = Horrorscope::putString;
    mFunctions.trigger_wait = Horrorscope::triggerWaitFunction;
    mFunctions.dma_read_buf = [](uint8_t* const, const uint16_t, const uint8_t) {};
    mFunctions.adc_gain = [](char){};
    mFunctions.adc_prescaler = [](char){};
    mFunctions.adc_calibrate = [](uint16_t){};
    mFunctions.set_sample_width = [](char) {};
    mFunctions.glitch_init = [](uint8_t){};
    mFunctions.glitch_execute = [](uint8_t){};
    mFunctions.turn_on = [] {};
    mFunctions.turn_off = [] {};

    mTim.registerInterruptCallback([&] {
        Trace(ZONE_INFO, "Timer\r\n");
        mDma.setupTransfer(mData.scopeBuf, sizeof(mData.scopeBuf) / 4, false);
        mDma.enable();
    });

    mDma.registerInterruptCallback([&] {
        Trace(ZONE_INFO, "Dma Transfer done\r\n");
        mCaptureDone = true;
        mDma.disable();
    }, hal::Dma::InterruptSource::TC);

    mTrigger.registerInterruptCallback([&] {
        Trace(ZONE_INFO, "Trigger\r\n");
        mTim.setCounterValue(mData.delay);
        mTim.enable();
        mTrigger.disable();
    });

    mAdc2.startConversion();
    mAdc1.startConversion();
    mTim.selectOnePulseMode(TIM_OPMode_Single);

    ::HorrorscopeExecute(&mData, &mFunctions);
}
