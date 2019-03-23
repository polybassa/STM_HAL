// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2019 Nils Weiss
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

typedef void (* HorrorscopeInitFunction)(void);
typedef char (* HorrorscopeUsbSerialGetCharFunction)(void);
typedef uint16_t (* HorrorscopeUsbSerialGetShortFunction)(void);
typedef void (* HorrorscopeUsbSerialPutCharFunction)(const char);
typedef void (* HorrorscopeUsbSerialPutBufferFunction)(uint8_t const* const, const uint16_t);
typedef void (* HorrorscopeUsbSerialPutStringFunction)(const char*);
typedef uint8_t (* HorrorscopeTriggerWaitFunction)(const uint32_t, const uint16_t);
typedef void (* HorrorscopeDmaReadBufferFunction)(uint8_t* const, const uint16_t, const uint8_t);
typedef void (* HorrorscopeAdcSetGainFunction)(char);
typedef void (* HorrorscopeAdcSetPrescalerFunction)(char);
typedef void (* HorrorscopeAdcCalibrateFunction)(uint16_t);
typedef void (* HorrorscopeSetSampleWidthFunction)(char);
typedef void (* HorrorscopeGlitchInitFunction)(uint8_t);
typedef void (* HorrorscopeGlitchExecuteFunction)(uint8_t);
typedef void (* HorrorscopeTurnOnFunction)(void);
typedef void (* HorrorscopeTurnOffFunction)(void);

typedef struct {
    HorrorscopeInitFunction init;
    HorrorscopeUsbSerialGetCharFunction usb_get_char;
    HorrorscopeUsbSerialGetShortFunction usb_get_short;
    HorrorscopeUsbSerialPutCharFunction usb_put_char;
    HorrorscopeUsbSerialPutBufferFunction usb_put_buffer;
    HorrorscopeUsbSerialPutStringFunction usb_put_string;
    HorrorscopeTriggerWaitFunction trigger_wait;
    HorrorscopeDmaReadBufferFunction dma_read_buf;
    HorrorscopeAdcSetGainFunction adc_gain;
    HorrorscopeAdcSetPrescalerFunction adc_prescaler;
    HorrorscopeAdcCalibrateFunction adc_calibrate;
    HorrorscopeSetSampleWidthFunction set_sample_width;
    HorrorscopeGlitchInitFunction glitch_init;
    HorrorscopeGlitchExecuteFunction glitch_execute;
    HorrorscopeTurnOnFunction turn_on;
    HorrorscopeTurnOffFunction turn_off;
} HorrorscopeFunctions;

typedef struct {
    uint8_t scopeBuf[4096];
    uint32_t timeout;
    uint16_t delay;
    uint16_t numberOfSamples;
    uint8_t glitch_length;
    uint8_t bits12;
    uint8_t pulses;
} HorrorscopeData;

void HorrorscopeExecute(HorrorscopeData* const, HorrorscopeFunctions* const);

#ifdef __cplusplus
}
#endif
