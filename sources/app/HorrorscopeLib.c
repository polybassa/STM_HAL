// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2019 Nils Weiss
 */

#include "HorrorscopeLib.h"
#include "string.h"
#include "stdint.h"

void HorrorscopeExecute(HorrorscopeData* const data, HorrorscopeFunctions* const func)
{
    uint8_t timedOut = 0;
    uint16_t temp = 0;
    data->timeout = 10000;
    data->delay = 100;
    data->numberOfSamples = sizeof(data->scopeBuf);
    data->glitch_length = 0;
    data->bits12 = 1;
    data->pulses = 1;

    memset(data->scopeBuf, 'U', sizeof(data->scopeBuf));
    func->init();

    while (1) {
        char command = func->usb_get_char();

        switch (command) {
        case 'T':
        case 't':
            func->usb_put_char('T');
            break;

        case 'd':
            //DMA ARM
            memset(data->scopeBuf, 0, sizeof(data->scopeBuf));
            func->usb_put_char('r');
            timedOut = func->trigger_wait(data->timeout, data->delay);
            func->dma_read_buf(data->scopeBuf, data->numberOfSamples, data->bits12);
            func->usb_put_char(timedOut ? 'e' : 'd');
            break;

        case 's'://Sample fetch
            func->usb_put_buffer(data->scopeBuf, data->numberOfSamples);
            break;

        case 'n':
            //Set number of samples
            temp = func->usb_get_short();

            if (temp > sizeof(data->scopeBuf)) {
                func->usb_put_string("Buffer too big\r\n");
                break;
            }

            data->numberOfSamples = temp;
            func->usb_put_char('d');
            break;

        case 'g'://Set gain
            func->adc_gain(func->usb_get_char() << 2);
            func->usb_put_char('d');
            break;

        case 'c'://Set clock divider ( values: 0-7 )
            func->adc_prescaler(func->usb_get_char() & 0x7);
            func->usb_put_char('d');
            break;

        case 'b'://Set bias...
            func->adc_calibrate(func->usb_get_short());
            func->usb_put_char('d');
            break;

        case 'f':
            //Fault (g is taken)
            func->usb_put_char('r');
            timedOut = func->trigger_wait(data->timeout, data->delay);
            func->usb_put_char(timedOut ? 'd' : 'e');//d = done;e=error/timeout
            break;

        case '8'://Definition of Bits12? fetch 2bytes: fetch the LSB
            data->bits12 = func->usb_get_char() & 1;
            func->set_sample_width(data->bits12);
            func->usb_put_char('d');
            break;

        case 'h':
            func->glitch_init(0xff - ((uint8_t)data->glitch_length));
            //Say we did the initialization
            func->usb_put_char('r');

            timedOut = func->trigger_wait(data->timeout, data->delay);
            func->glitch_execute(data->pulses);
            func->usb_put_char(timedOut ? 'e' : 'd');
            break;

        case 'w': //Wait (d is taken)
            data->delay = func->usb_get_short();
            func->usb_put_char('d');
            break;

        case 'l': //glitch Length
            data->glitch_length = func->usb_get_short();
            func->usb_put_char('d');
            break;

        case 'e': //Exit timeout t was taken
            data->timeout = func->usb_get_short();
            func->usb_put_char('d');
            break;

        case 'p': //Exit timeout t was taken
            data->pulses = func->usb_get_char();
            func->usb_put_char('d');
            break;

        case 'x': //Turn off
            func->turn_off();
            func->usb_put_char('d');
            break;

        case 'y': //turn on
            func->turn_on();
            func->usb_put_char('d');
            break;

        default:
            func->usb_put_string("Invalid command:");
            func->usb_put_char(command);
            func->usb_put_string("\r\n");
            break;
        }
    }
}
