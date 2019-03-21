// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "TestUSB.h"

#include "trace.h"
#include <cstring>
#include <cstdio>
#include "stm32f10x_adc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_exti.h"

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

static uint32_t storage[2000] = {0};
static bool captureDone;
size_t numberOfSamples = sizeof(storage) / sizeof(uint32_t);
size_t timeout = 10000;
size_t pulses = 10000;
size_t delay = 1;
size_t bits12 = 0;
size_t glitch_length = 0;

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

extern "C" void DMA1_Channel1_IRQHandler(void)
{
    if (DMA_GetITStatus(DMA1_IT_TC1)) {
        DMA_ClearITPendingBit(DMA1_IT_TC1);
        Trace(ZONE_INFO, "DMA IRG TC\r\n");
        captureDone = true;
        DMA_Cmd(DMA1_Channel1, DISABLE);
    }

    if (DMA_GetITStatus(DMA1_IT_GL1)) {
        DMA_ClearITPendingBit(DMA1_IT_GL1);
        Trace(ZONE_INFO, "DMA IRG GL\r\n");
    }
    if (DMA_GetITStatus(DMA1_IT_HT1)) {
        DMA_ClearITPendingBit(DMA1_IT_HT1);
        Trace(ZONE_INFO, "DMA IRG HT\r\n");
    }

    if (DMA_GetITStatus(DMA1_IT_TE1)) {
        DMA_ClearITPendingBit(DMA1_IT_TE1);
        Trace(ZONE_INFO, "DMA IRG TE\r\n");
    }
}

void setupTimer(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    TIM_Cmd(TIM3, DISABLE);

    TIM_TimeBaseInitTypeDef timinit;
    timinit.TIM_Prescaler = 360;
    timinit.TIM_CounterMode = TIM_CounterMode_Down;
    timinit.TIM_Period = 1;
    timinit.TIM_ClockDivision = 0;
    timinit.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM3, &timinit);
    TIM_SelectOnePulseMode(TIM3, TIM_OPMode_Single);
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    NVIC_EnableIRQ(TIM3_IRQn);
}

void setupADC(void)
{
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);

    ADC_InitTypeDef initdef;
    initdef.ADC_Mode = ADC_Mode_SlowInterl;
    initdef.ADC_ScanConvMode = DISABLE;
    initdef.ADC_ContinuousConvMode = DISABLE;
    initdef.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;

    initdef.ADC_DataAlign = ADC_DataAlign_Right;
    initdef.ADC_NbrOfChannel = 1;

    ADC_Init(ADC1, &initdef);
    initdef.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;

    ADC_Init(ADC2, &initdef);
    ADC_SoftwareStartConvCmd(ADC2, ENABLE);

    ADC_DMACmd(ADC1, ENABLE);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_13Cycles5);
    ADC_RegularChannelConfig(ADC2, ADC_Channel_0, 2, ADC_SampleTime_13Cycles5);

    ADC_Cmd(ADC1, ENABLE);
    ADC_Cmd(ADC2, ENABLE);

    /* Enable ADC1 reset calibration register */
    ADC_ResetCalibration(ADC1);
    /* Check the end of ADC1 reset calibration register */
    while (ADC_GetResetCalibrationStatus(ADC1)) {
        os::ThisTask::sleep(std::chrono::milliseconds(1));
    }

    /* Start ADC1 calibration */
    ADC_StartCalibration(ADC1);
    /* Check the end of ADC1 calibration */
    while (ADC_GetCalibrationStatus(ADC1)) {
        os::ThisTask::sleep(std::chrono::milliseconds(1));
    }

    /* Enable ADC1 reset calibration register */
    ADC_ResetCalibration(ADC2);
    /* Check the end of ADC1 reset calibration register */
    while (ADC_GetResetCalibrationStatus(ADC2)) {
        os::ThisTask::sleep(std::chrono::milliseconds(1));
    }

    /* Start ADC1 calibration */
    ADC_StartCalibration(ADC2);
    /* Check the end of ADC1 calibration */
    while (ADC_GetCalibrationStatus(ADC2)) {
        os::ThisTask::sleep(std::chrono::milliseconds(1));
    }
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void setupDMA(void)
{
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    DMA_ITConfig(DMA1_Channel1, DMA1_IT_TC1, ENABLE);
    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

void setupEXTI(void)
{
    EXTI_ClearITPendingBit(EXTI_Line3);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource3);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource3);
    NVIC_EnableIRQ(EXTI3_IRQn);
}

void enableEXTI()
{
    EXTI_ClearITPendingBit(EXTI_Line3);

    EXTI_InitTypeDef extiinit;
    extiinit.EXTI_Line = EXTI_Line3;
    extiinit.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    extiinit.EXTI_Mode = EXTI_Mode_Interrupt;
    extiinit.EXTI_LineCmd = ENABLE;
    EXTI_Init(&extiinit);
}

void disableEXTI(void)
{
    EXTI_InitTypeDef extiinit;
    extiinit.EXTI_Line = EXTI_Line3;
    extiinit.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    extiinit.EXTI_Mode = EXTI_Mode_Interrupt;
    extiinit.EXTI_LineCmd = DISABLE;
    EXTI_Init(&extiinit);
}

void startDMA(uint32_t* src = (uint32_t*)(&(ADC1->DR)), uint32_t* dst = storage, size_t len = sizeof(storage))
{
    DMA_Cmd(DMA1_Channel1, DISABLE);

    DMA_InitTypeDef dmainit;
    dmainit.DMA_PeripheralBaseAddr = (uint32_t)src;
    dmainit.DMA_MemoryBaseAddr = (uint32_t)dst;
    dmainit.DMA_DIR = DMA_DIR_PeripheralSRC;
    dmainit.DMA_BufferSize = len / 4;
    dmainit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    dmainit.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dmainit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    dmainit.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    dmainit.DMA_Mode = DMA_Mode_Normal;
    dmainit.DMA_Priority = DMA_Priority_VeryHigh;
    dmainit.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMA1_Channel1, &dmainit);
}

void startTimer(uint16_t preload = 100)
{
    TIM_SetCounter(TIM3, preload);
    TIM_Cmd(TIM3, ENABLE);
}

extern "C" void TIM3_IRQHandler(void)
{
    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    Trace(ZONE_INFO, "TIM IRG \r\n");

    DMA_Cmd(DMA1_Channel1, ENABLE);
    TIM_Cmd(TIM3, DISABLE);
}

extern "C" void EXTI3_IRQHandler(void)
{
    Trace(ZONE_INFO, "EXTI IRG \r\n");

    EXTI_ClearITPendingBit(EXTI_Line3);

    startDMA();
    startTimer(delay);
    disableEXTI();
}

extern "C" void horrorscopeSetup(void)
{
    setupTimer();
//------------------------------------------------------------
    setupADC();
//----------------------------------------------------------------
    setupDMA();
    setupEXTI();
}

const os::TaskEndless usbTest("usb_Test",
                              2048, os::Task::Priority::HIGH,
                              [](const bool&){
                              Trace(ZONE_INFO, "Hallo MainUSB\r\n");
                              ::horrorscopeSetup();

                              unsigned char len;
                              uint32_t val;
                              const size_t blocklength = 58;
                              uint8_t buffer[blocklength];

                              while (1) {
                                  //memset(Receive_Buffer, 0, sizeof(Receive_Buffer));

                                  if (bDeviceState != CONFIGURED) {
                                      continue;
                                  }
                                  CDC_Receive_DATA();
                                  os::ThisTask::sleep(std::chrono::milliseconds(1));

                                  // Check to see if we have data yet
                                  if ((Receive_length != 0) && (packet_sent == 1)) {
                                      Receive_Buffer[Receive_length] = 0;
                                      Trace(ZONE_INFO, "%s\r\n", Receive_Buffer);

                                      char command = Receive_Buffer[0];

                                      switch (command) {
                                      case 'T':
                                      case 't':
                                          CDC_Send_DATA((uint8_t*)"T", 1);
                                          break;

                                      case 'd':
                                          //DMA ARM
                                          memset(storage, 0, sizeof(storage));
                                          CDC_Send_DATA((uint8_t*)"r", 1);
                                          captureDone = false;
                                          enableEXTI();

                                          //EXTI3_IRQHandler();
                                          Trace(ZONE_INFO, "Armed\r\n");

                                          for (size_t i = 0; (i < timeout) && captureDone == false; i++) {
                                              os::ThisTask::sleep(std::chrono::milliseconds(1));
                                          }
                                          Trace(ZONE_INFO, captureDone ? "captured\r\n" : "timeout\r\n");

                                          CDC_Send_DATA(captureDone ? (uint8_t*)"d" : (uint8_t*)"e", 1);
                                          break;

                                      case 's'://Sample fetch

                                          for (size_t i = 0; i < sizeof(storage) / blocklength; i++) {
                                              SEGGER_RTT_printf(0, "sending %d\r\n", i);
                                              memcpy(buffer, ((uint8_t*)storage + i* blocklength), blocklength);

                                              CDC_Send_DATA(buffer,
                                                            blocklength);

                                              while (packet_sent == 0) {
                                                  os::ThisTask::sleep(std::chrono::milliseconds(1));
                                              }
                                          }
                                          CDC_Send_DATA(
                                                        ((uint8_t*)storage + sizeof(storage) - (sizeof(storage) % blocklength)),
                                                        sizeof(storage) % blocklength);

                                          break;

                                      case 'n':
                                          {
                                              uint16_t temp = Receive_Buffer[1] << 8 | Receive_Buffer[2];

                                              if (temp > sizeof(storage)) {
                                                  CDC_Send_DATA((uint8_t*)"Buffer too big\r\n", 16);
                                                  break;
                                              }

                                              numberOfSamples = temp;
                                              CDC_Send_DATA((uint8_t*)"d", 1);
                                          }
                                          break;

                                      case 'g'://Set gain
                                          //adc_ch_gain(&(ADCA.CH0), USB_serial_get_char() << 2);
                                          CDC_Send_DATA((uint8_t*)"d", 1);
                                          break;

                                      case 'c'://Set clock divider ( values: 0-7 )
                                          //adc_prescaler(USB_serial_get_char() & 0x7);
                                          CDC_Send_DATA((uint8_t*)"d", 1);
                                          break;

                                      case 'b'://Set bias...
                                          //adc_calibrate(USB_serial_get_short());
                                          CDC_Send_DATA((uint8_t*)"d", 1);
                                          break;

                                      case 'f':
                                          {  //Fault (g is taken)
                                              CDC_Send_DATA((uint8_t*)"r", 1);

                                              uint8_t timedOut = 0; //trigger_wait(timeout, delay);
                                              //glitch(0,glitch_length,0xff);
                                              CDC_Send_DATA(timedOut ? (uint8_t*)"e" : (uint8_t*)"d", 1);
                                              break;
                                          }

                                      case '8'://Definition of Bits12? fetch 2bytes: fetch the LSB
                                          bits12 = Receive_Buffer[1] & 1;
                                          //adc_resolution(ADC_RESOLUTION_12BIT_gc);
                                          //dmaadc_init(bits12);
                                          //trigger_init(); //As the dmaadc reconfigures the DMA controller
                                          CDC_Send_DATA((uint8_t*)"d", 1);
                                          break;

                                      case 'h':
                                          {
                                              //Initialization
                                              //hires_set_counter(0x00);
                                              //hires_set_compare(0xff - ((uint8_t)glitch_length));

                                              //Say we did the initialization
                                              CDC_Send_DATA((uint8_t*)"r", 1);

                                              uint8_t timedOut = 0;//trigger_wait(timeout, delay);
                                              //Do the glitch
                                              //for (int i = 0; i < pulses; i++) {
                                              //    hires_set_counter(0x00);
                                              //    hires_glitch();
                                              //}

                                              CDC_Send_DATA(timedOut ? (uint8_t*)"e" : (uint8_t*)"d", 1);
                                              break;
                                          }

                                      case 'w': //Wait (d is taken)
                                          delay = Receive_Buffer[1] << 8 | Receive_Buffer[2];
                                          CDC_Send_DATA((uint8_t*)"d", 1);
                                          break;

                                      case 'l': //glitch Length
                                          glitch_length = Receive_Buffer[1] << 8 | Receive_Buffer[2];
                                          CDC_Send_DATA((uint8_t*)"d", 1);
                                          break;

                                      case 'e': //Exit timeout t was taken
                                          timeout = Receive_Buffer[1] << 8 | Receive_Buffer[2];
                                          CDC_Send_DATA((uint8_t*)"d", 1);
                                          break;

                                      case 'p': //Exit timeout t was taken
                                          pulses = Receive_Buffer[1];
                                          CDC_Send_DATA((uint8_t*)"d", 1);
                                          break;

                                      case 'x': //Turn off
                                          //PORTC.OUTCLR = 0xff;
                                          //TCC2.CTRLC = 0x00; //This is used when TCC2 override is enabled
                                          CDC_Send_DATA((uint8_t*)"d", 1);
                                          break;

                                      case 'y': //turn on
                                          //PORTC.OUTSET = 0xff;
                                          //TCC2.CTRLC = 0xff;
                                          CDC_Send_DATA((uint8_t*)"d", 1);
                                          break;

                                      default:
                                          len = sprintf((char*)buffer,
                                                        "Invalid command: %c \r\n",
                                                        command);
                                          CDC_Send_DATA(buffer, len);
                                          break;
                                      }

                                      Receive_length = 0;
                                  }
                              }
    }
                              );
