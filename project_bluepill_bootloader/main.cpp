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

/* GENERAL INCLUDES */
#include "os_Task.h"
#include "cpp_overrides.h"
#include "trace.h"
extern "C" {
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_mem.h"
#include "hw_config.h"
}

/* OS LAYER INCLUDES */
#include "hal_Factory.h"
#include "Gpio.h"
#include "stm32f10x_dbgmcu.h"

/* DEV LAYER INLCUDES */

/* VIRT LAYER INCLUDES */

/* COM LAYER INCLUDES */

/* APP LAYER INLCUDES */

/* GLOBAL VARIABLES */
static const int __attribute__((used)) g_DebugZones = ZONE_ERROR | ZONE_WARNING |
                                                      ZONE_VERBOSE | ZONE_INFO;

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

extern "C" void PC_Check(void);

extern "C" void BLSystemInit(void)
{
    /* Reset the RCC clock configuration to the default reset state(for debug purpose) */
    /* Set HSION bit */
    AFIO->MAPR = 0x04000000;

    RCC->CR = (uint32_t)0x00000001;

    /* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits */
    RCC->CFGR = (uint32_t)0x00000000;

    /* Disable all interrupts and clear pending bits  */
    RCC->CIR = 0x009F0000;

    SCB->VTOR = FLASH_BASE; /* Vector Table Relocation in Internal FLASH. */

    RCC_APB2PeriphResetCmd(0x0038fffd, ENABLE);
    RCC_APB1PeriphResetCmd(0x3afec9ff, ENABLE);
    AFIO->MAPR = 0x04000000;

    RCC_APB2PeriphResetCmd(0x0038fffd, DISABLE);
    RCC_APB1PeriphResetCmd(0x3afec9ff, DISABLE);
    AFIO->MAPR = 0x04000000;

    DBGMCU_Config(0x7e3fffe7, DISABLE);
    PC_Check();
}

extern char _version_start;
extern char _version_end;
const std::string_view VERSION(&_version_start, (&_version_end - &_version_start));

int main(void)
{
    SystemInit();
    hal::initFactory<hal::Factory<hal::Gpio> >();

    TraceInit();
    Trace(ZONE_INFO, "Version: %s \r\n", VERSION.data());

    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();
}

void assert_failed(uint8_t* file, uint32_t line)
{
    Trace(ZONE_ERROR, "ASSERT FAILED: %s:%u", file, line);
}
