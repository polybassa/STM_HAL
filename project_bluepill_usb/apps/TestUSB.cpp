// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "TestUSB.h"

#include "trace.h"
#include <cstring>
#include <cstdio>

extern "C" {
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_mem.h"
#include "hw_config.h"
}

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

uint8_t Receive_Buffer[64];
uint32_t Send_length;

extern volatile uint32_t Receive_length;
uint8_t Send_Buffer[64];
uint32_t packet_sent = 1;
uint32_t packet_receive = 1;

uint32_t CDC_Send_DATA(uint8_t* ptrBuffer, uint8_t Send_length)
{
    /*if max buffer is Not reached*/
    if (Send_length < VIRTUAL_COM_PORT_DATA_SIZE) {
        /*Sent flag*/
        packet_sent = 0;
        /* send  packet to PMA*/
        UserToPMABufferCopy((uint8_t*)ptrBuffer, ENDP1_TXADDR, Send_length);
        SetEPTxCount(ENDP1, Send_length);
        SetEPTxValid(ENDP1);
    } else {
        return 0;
    }
    return 1;
}

uint32_t CDC_Receive_DATA(void)
{
    /*Receive flag*/
    packet_receive = 0;
    SetEPRxValid(ENDP3);
    return 1;
}

const os::TaskEndless usbTest("usb_Test",
                              1024, os::Task::Priority::HIGH,
                              [](const bool&){
                              Trace(ZONE_INFO, "Hallo MainUSB\r\n");
                              while (true) {
                                  os::ThisTask::sleep(std::chrono::milliseconds(1));

                                  if (bDeviceState == CONFIGURED) {
                                      CDC_Receive_DATA();
                                      // Check to see if we have data yet
                                      if (Receive_length != 0) {
                                          // Echo
                                          if (packet_sent == 1) {
                                              uint8_t buffer[32];
                                              Receive_Buffer[Receive_length] = 0;
                                              unsigned char len = sprintf((char*)buffer, "GOT: %s \r\n",
                                                                          Receive_Buffer);
                                              CDC_Send_DATA(buffer, len);
                                          }
                                          Receive_length = 0;
                                      }
                                  }
                              }
    });
