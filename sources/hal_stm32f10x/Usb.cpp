// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include <algorithm>
#include <cstring>

#include "Usb.h"
#include "trace.h"
#include "os_Task.h"

extern "C" {
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_mem.h"
#include "hw_config.h"
}

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Factory;
using hal::Usb;

extern volatile uint32_t Receive_length;
size_t Receive_index;
uint8_t Receive_Buffer[64];
uint32_t packet_sent = 1;
uint32_t packet_receive = 1;

void Usb::initialize() const
{
    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();
    packet_sent = 1;
    packet_receive = 0;
    Receive_index = 0;
}

bool Usb::isConfigured(void) const
{
    return bDeviceState == CONFIGURED;
}

size_t Usb::send(const std::string_view& str) const
{
    return send(reinterpret_cast<uint8_t const* const>(str.data()), str.length());
}

size_t Usb::send(uint8_t const* const data, const size_t length, const std::chrono::milliseconds timeout) const
{
    if (data == nullptr) {
        return 0;
    }
    if (!isConfigured()) {
        return 0;
    }

    size_t bytesSend = 0;
    size_t timeSpent = 0;

    while (bytesSend < length) {
        if (this->isReadyToSend()) {
            packet_sent = 0;
            const size_t chunkLength = std::min((size_t)(VIRTUAL_COM_PORT_DATA_SIZE), length - bytesSend);
            UserToPMABufferCopy((unsigned char*)data + bytesSend, ENDP1_TXADDR, chunkLength);
            SetEPTxCount(ENDP1, chunkLength);
            SetEPTxValid(ENDP1);
            bytesSend += chunkLength;
        }

        if ((bytesSend == length) || (timeSpent >= timeout.count())) {
            return bytesSend;
        }
        timeSpent++;
        os::ThisTask::sleep(std::chrono::milliseconds(1));
    }

    return bytesSend;
}

bool Usb::isReadyToReceive(void) const
{
    if (Receive_length > 0) {
        return true;
    }
    packet_receive = 0;
    Receive_index = 0;
    SetEPRxValid(ENDP3);
    return Receive_length > 0;
}

void Usb::resetRxBuffer(void) const
{
    Receive_length = 0;
}

size_t Usb::receive(uint8_t* const data, const size_t length, const std::chrono::milliseconds timeout) const
{
    if (data == nullptr) {
        return 0;
    }

    size_t bytesReceived = 0;
    size_t timeSpent = 0;
    while (bytesReceived < length) {
        if (isReadyToReceive()) {
            size_t recvlen = std::min(length, (size_t)Receive_length);
            std::memcpy(&data[bytesReceived], Receive_Buffer + Receive_index, recvlen);
            bytesReceived += recvlen;
            Receive_index += recvlen;
            Receive_length -= recvlen;
        }
        if ((bytesReceived == length) || (timeSpent >= timeout.count())) {
            return bytesReceived;
        }
        timeSpent++;
        os::ThisTask::sleep(std::chrono::milliseconds(1));
    }
    return bytesReceived;
}

size_t Usb::receiveAvailableData(uint8_t* const data, const size_t length) const
{
    return receive(data, length, std::chrono::milliseconds(0));
}

bool Usb::isReadyToSend(void) const
{
    return packet_sent == 1 ? true : false;
}
