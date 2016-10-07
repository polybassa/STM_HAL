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
/**
  ******************************************************************************
  * @file      startup_stm32f30x.s
  * @author    MCD Application Team
  * @version   V1.2.2
  * @date      27-February-2015
  * @brief     STM32F30x Devices vector table for RIDE7 toolchain. 
  *            This module performs:
  *                - Set the initial SP
  *                - Set the initial PC == Reset_Handler,
  *                - Set the vector table entries with the exceptions ISR address
  *                - Configure the clock system and the external SRAM mounted on 
  *                  STM3230C-EVAL board to be used as data memory (optional, 
  *                  to be enabled by user)
  *                - Branches to main in the C library (which eventually
  *                  calls main()).
  *            After Reset the Cortex-M4 processor is in Thread mode,
  *            priority is Privileged, and the Stack is set to Main.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */
    
  .syntax unified
  .cpu cortex-m4
  .fpu softvfp
  .thumb

.global  g_pfnVectors
.global  Default_Handler

/* start address for the initialization values of the .data section. 
defined in linker script */
.word  _sidata
/* start address for the .data section. defined in linker script */  
.word  _sdata
/* end address for the .data section. defined in linker script */
.word  _edata
/* start address for the .bss section. defined in linker script */
.word  _sbss
/* end address for the .bss section. defined in linker script */
.word  _ebss
/* stack used for SystemInit_ExtMemCtl; always internal RAM used */

/**
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called. 
 * @param  None
 * @retval : None
*/

    .section  .text.Reset_Handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:  

/* Copy the data segment initializers from flash to SRAM */  
  movs  r1, #0
  b  LoopCopyDataInit

CopyDataInit:
  ldr  r3, =_sidata
  ldr  r3, [r3, r1]
  str  r3, [r0, r1]
  adds  r1, r1, #4
    
LoopCopyDataInit:
  ldr  r0, =_sdata
  ldr  r3, =_edata
  adds  r2, r0, r1
  cmp  r2, r3
  bcc  CopyDataInit
  ldr  r2, =_sbss
  b  LoopFillZerobss
/* Zero fill the bss segment. */  
FillZerobss:
  movs  r3, #0
  str  r3, [r2], #4
    
LoopFillZerobss:
  ldr  r3, = _ebss
  cmp  r2, r3
  bcc  FillZerobss

  ldr  r2, =__heap_start
  b  LoopFillHeap
/* Zero fill the bss segment. */
FillHeap:
  movs  r3, #0x00000000
  str  r3, [r2], #4

LoopFillHeap:
  ldr  r3, =__heap_end
  cmp  r2, r3
  bcc  FillHeap

/* Call the clock system initialization function.*/
  bl  SystemInit   

#ifdef SYSVIEW
/* INIT SEGGER SYSTEMVIEW */
  bl  SEGGER_SYSVIEW_Conf
  bl  SEGGER_SYSVIEW_Start
#endif /* SYSVIEW */

/* Call static constructors */
  bl __libc_init_array
/* Call the application's entry point.*/
  bl  main
  bx  lr    
.size  Reset_Handler, .-Reset_Handler

/**
 * @brief  This is the code that gets called when the processor receives an 
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 * @param  None     
 * @retval None       
*/
    .section  .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
  b  Infinite_Loop
  .size  Default_Handler, .-Default_Handler


#ifdef SYSVIEW

/**
 * @brief  This are special SYSVIEW interrupthandler to enable tracing of interruptcalls.
 * @param  None
 * @retval None
*/
.section  .text.WWDG_IRQHandlerSV
.weak  WWDG_IRQHandlerSV
.type WWDG_IRQHandlerSV, %function
WWDG_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl WWDG_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  WWDG_IRQHandlerSV, .-WWDG_IRQHandlerSV


.section  .text.PVD_IRQHandlerSV
.weak  PVD_IRQHandlerSV
.type PVD_IRQHandlerSV, %function
PVD_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl PVD_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  PVD_IRQHandlerSV, .-PVD_IRQHandlerSV


.section  .text.TAMPER_STAMP_IRQHandlerSV
.weak  TAMPER_STAMP_IRQHandlerSV
.type TAMPER_STAMP_IRQHandlerSV, %function
TAMPER_STAMP_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl TAMPER_STAMP_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  TAMPER_STAMP_IRQHandlerSV, .-TAMPER_STAMP_IRQHandlerSV


.section  .text.RTC_WKUP_IRQHandlerSV
.weak  RTC_WKUP_IRQHandlerSV
.type RTC_WKUP_IRQHandlerSV, %function
RTC_WKUP_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl RTC_WKUP_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  RTC_WKUP_IRQHandlerSV, .-RTC_WKUP_IRQHandlerSV


.section  .text.FLASH_IRQHandlerSV
.weak  FLASH_IRQHandlerSV
.type FLASH_IRQHandlerSV, %function
FLASH_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl FLASH_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  FLASH_IRQHandlerSV, .-FLASH_IRQHandlerSV


.section  .text.RCC_IRQHandlerSV
.weak  RCC_IRQHandlerSV
.type RCC_IRQHandlerSV, %function
RCC_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl RCC_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  RCC_IRQHandlerSV, .-RCC_IRQHandlerSV


.section  .text.EXTI0_IRQHandlerSV
.weak  EXTI0_IRQHandlerSV
.type EXTI0_IRQHandlerSV, %function
EXTI0_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl EXTI0_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  EXTI0_IRQHandlerSV, .-EXTI0_IRQHandlerSV


.section  .text.EXTI1_IRQHandlerSV
.weak  EXTI1_IRQHandlerSV
.type EXTI1_IRQHandlerSV, %function
EXTI1_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl EXTI1_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  EXTI1_IRQHandlerSV, .-EXTI1_IRQHandlerSV


.section  .text.EXTI2_TS_IRQHandlerSV
.weak  EXTI2_TS_IRQHandlerSV
.type EXTI2_TS_IRQHandlerSV, %function
EXTI2_TS_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl EXTI2_TS_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  EXTI2_TS_IRQHandlerSV, .-EXTI2_TS_IRQHandlerSV


.section  .text.EXTI3_IRQHandlerSV
.weak  EXTI3_IRQHandlerSV
.type EXTI3_IRQHandlerSV, %function
EXTI3_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl EXTI3_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  EXTI3_IRQHandlerSV, .-EXTI3_IRQHandlerSV


.section  .text.EXTI4_IRQHandlerSV
.weak  EXTI4_IRQHandlerSV
.type EXTI4_IRQHandlerSV, %function
EXTI4_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl EXTI4_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  EXTI4_IRQHandlerSV, .-EXTI4_IRQHandlerSV


.section  .text.DMA1_Channel1_IRQHandlerSV
.weak  DMA1_Channel1_IRQHandlerSV
.type DMA1_Channel1_IRQHandlerSV, %function
DMA1_Channel1_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl DMA1_Channel1_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  DMA1_Channel1_IRQHandlerSV, .-DMA1_Channel1_IRQHandlerSV


.section  .text.DMA1_Channel2_IRQHandlerSV
.weak  DMA1_Channel2_IRQHandlerSV
.type DMA1_Channel2_IRQHandlerSV, %function
DMA1_Channel2_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl DMA1_Channel2_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  DMA1_Channel2_IRQHandlerSV, .-DMA1_Channel2_IRQHandlerSV


.section  .text.DMA1_Channel3_IRQHandlerSV
.weak  DMA1_Channel3_IRQHandlerSV
.type DMA1_Channel3_IRQHandlerSV, %function
DMA1_Channel3_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl DMA1_Channel3_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  DMA1_Channel3_IRQHandlerSV, .-DMA1_Channel3_IRQHandlerSV


.section  .text.DMA1_Channel4_IRQHandlerSV
.weak  DMA1_Channel4_IRQHandlerSV
.type DMA1_Channel4_IRQHandlerSV, %function
DMA1_Channel4_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl DMA1_Channel4_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  DMA1_Channel4_IRQHandlerSV, .-DMA1_Channel4_IRQHandlerSV


.section  .text.DMA1_Channel5_IRQHandlerSV
.weak  DMA1_Channel5_IRQHandlerSV
.type DMA1_Channel5_IRQHandlerSV, %function
DMA1_Channel5_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl DMA1_Channel5_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  DMA1_Channel5_IRQHandlerSV, .-DMA1_Channel5_IRQHandlerSV


.section  .text.DMA1_Channel6_IRQHandlerSV
.weak  DMA1_Channel6_IRQHandlerSV
.type DMA1_Channel6_IRQHandlerSV, %function
DMA1_Channel6_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl DMA1_Channel6_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  DMA1_Channel6_IRQHandlerSV, .-DMA1_Channel6_IRQHandlerSV


.section  .text.DMA1_Channel7_IRQHandlerSV
.weak  DMA1_Channel7_IRQHandlerSV
.type DMA1_Channel7_IRQHandlerSV, %function
DMA1_Channel7_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl DMA1_Channel7_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  DMA1_Channel7_IRQHandlerSV, .-DMA1_Channel7_IRQHandlerSV


.section  .text.ADC1_2_IRQHandlerSV
.weak  ADC1_2_IRQHandlerSV
.type ADC1_2_IRQHandlerSV, %function
ADC1_2_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl ADC1_2_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  ADC1_2_IRQHandlerSV, .-ADC1_2_IRQHandlerSV


.section  .text.USB_HP_CAN1_TX_IRQHandlerSV
.weak  USB_HP_CAN1_TX_IRQHandlerSV
.type USB_HP_CAN1_TX_IRQHandlerSV, %function
USB_HP_CAN1_TX_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl USB_HP_CAN1_TX_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  USB_HP_CAN1_TX_IRQHandlerSV, .-USB_HP_CAN1_TX_IRQHandlerSV


.section  .text.USB_LP_CAN1_RX0_IRQHandlerSV
.weak  USB_LP_CAN1_RX0_IRQHandlerSV
.type USB_LP_CAN1_RX0_IRQHandlerSV, %function
USB_LP_CAN1_RX0_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl USB_LP_CAN1_RX0_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  USB_LP_CAN1_RX0_IRQHandlerSV, .-USB_LP_CAN1_RX0_IRQHandlerSV


.section  .text.CAN1_RX1_IRQHandlerSV
.weak  CAN1_RX1_IRQHandlerSV
.type CAN1_RX1_IRQHandlerSV, %function
CAN1_RX1_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl CAN1_RX1_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  CAN1_RX1_IRQHandlerSV, .-CAN1_RX1_IRQHandlerSV


.section  .text.CAN1_SCE_IRQHandlerSV
.weak  CAN1_SCE_IRQHandlerSV
.type CAN1_SCE_IRQHandlerSV, %function
CAN1_SCE_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl CAN1_SCE_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  CAN1_SCE_IRQHandlerSV, .-CAN1_SCE_IRQHandlerSV


.section  .text.EXTI9_5_IRQHandlerSV
.weak  EXTI9_5_IRQHandlerSV
.type EXTI9_5_IRQHandlerSV, %function
EXTI9_5_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl EXTI9_5_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  EXTI9_5_IRQHandlerSV, .-EXTI9_5_IRQHandlerSV


.section  .text.TIM1_BRK_TIM15_IRQHandlerSV
.weak  TIM1_BRK_TIM15_IRQHandlerSV
.type TIM1_BRK_TIM15_IRQHandlerSV, %function
TIM1_BRK_TIM15_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl TIM1_BRK_TIM15_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  TIM1_BRK_TIM15_IRQHandlerSV, .-TIM1_BRK_TIM15_IRQHandlerSV


.section  .text.TIM1_UP_TIM16_IRQHandlerSV
.weak  TIM1_UP_TIM16_IRQHandlerSV
.type TIM1_UP_TIM16_IRQHandlerSV, %function
TIM1_UP_TIM16_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl TIM1_UP_TIM16_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  TIM1_UP_TIM16_IRQHandlerSV, .-TIM1_UP_TIM16_IRQHandlerSV


.section  .text.TIM1_TRG_COM_TIM17_IRQHandlerSV
.weak  TIM1_TRG_COM_TIM17_IRQHandlerSV
.type TIM1_TRG_COM_TIM17_IRQHandlerSV, %function
TIM1_TRG_COM_TIM17_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl TIM1_TRG_COM_TIM17_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  TIM1_TRG_COM_TIM17_IRQHandlerSV, .-TIM1_TRG_COM_TIM17_IRQHandlerSV


.section  .text.TIM1_CC_IRQHandlerSV
.weak  TIM1_CC_IRQHandlerSV
.type TIM1_CC_IRQHandlerSV, %function
TIM1_CC_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl TIM1_CC_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  TIM1_CC_IRQHandlerSV, .-TIM1_CC_IRQHandlerSV


.section  .text.TIM2_IRQHandlerSV
.weak  TIM2_IRQHandlerSV
.type TIM2_IRQHandlerSV, %function
TIM2_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl TIM2_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  TIM2_IRQHandlerSV, .-TIM2_IRQHandlerSV


.section  .text.TIM3_IRQHandlerSV
.weak  TIM3_IRQHandlerSV
.type TIM3_IRQHandlerSV, %function
TIM3_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl TIM3_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  TIM3_IRQHandlerSV, .-TIM3_IRQHandlerSV


.section  .text.TIM4_IRQHandlerSV
.weak  TIM4_IRQHandlerSV
.type TIM4_IRQHandlerSV, %function
TIM4_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl TIM4_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  TIM4_IRQHandlerSV, .-TIM4_IRQHandlerSV


.section  .text.I2C1_EV_IRQHandlerSV
.weak  I2C1_EV_IRQHandlerSV
.type I2C1_EV_IRQHandlerSV, %function
I2C1_EV_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl I2C1_EV_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  I2C1_EV_IRQHandlerSV, .-I2C1_EV_IRQHandlerSV


.section  .text.I2C1_ER_IRQHandlerSV
.weak  I2C1_ER_IRQHandlerSV
.type I2C1_ER_IRQHandlerSV, %function
I2C1_ER_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl I2C1_ER_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  I2C1_ER_IRQHandlerSV, .-I2C1_ER_IRQHandlerSV


.section  .text.I2C2_EV_IRQHandlerSV
.weak  I2C2_EV_IRQHandlerSV
.type I2C2_EV_IRQHandlerSV, %function
I2C2_EV_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl I2C2_EV_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  I2C2_EV_IRQHandlerSV, .-I2C2_EV_IRQHandlerSV


.section  .text.I2C2_ER_IRQHandlerSV
.weak  I2C2_ER_IRQHandlerSV
.type I2C2_ER_IRQHandlerSV, %function
I2C2_ER_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl I2C2_ER_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  I2C2_ER_IRQHandlerSV, .-I2C2_ER_IRQHandlerSV


.section  .text.SPI1_IRQHandlerSV
.weak  SPI1_IRQHandlerSV
.type SPI1_IRQHandlerSV, %function
SPI1_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl SPI1_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  SPI1_IRQHandlerSV, .-SPI1_IRQHandlerSV


.section  .text.SPI2_IRQHandlerSV
.weak  SPI2_IRQHandlerSV
.type SPI2_IRQHandlerSV, %function
SPI2_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl SPI2_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  SPI2_IRQHandlerSV, .-SPI2_IRQHandlerSV


.section  .text.USART1_IRQHandlerSV
.weak  USART1_IRQHandlerSV
.type USART1_IRQHandlerSV, %function
USART1_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl USART1_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  USART1_IRQHandlerSV, .-USART1_IRQHandlerSV


.section  .text.USART2_IRQHandlerSV
.weak  USART2_IRQHandlerSV
.type USART2_IRQHandlerSV, %function
USART2_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl USART2_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  USART2_IRQHandlerSV, .-USART2_IRQHandlerSV


.section  .text.USART3_IRQHandlerSV
.weak  USART3_IRQHandlerSV
.type USART3_IRQHandlerSV, %function
USART3_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl USART3_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  USART3_IRQHandlerSV, .-USART3_IRQHandlerSV


.section  .text.EXTI15_10_IRQHandlerSV
.weak  EXTI15_10_IRQHandlerSV
.type EXTI15_10_IRQHandlerSV, %function
EXTI15_10_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl EXTI15_10_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  EXTI15_10_IRQHandlerSV, .-EXTI15_10_IRQHandlerSV


.section  .text.RTC_Alarm_IRQHandlerSV
.weak  RTC_Alarm_IRQHandlerSV
.type RTC_Alarm_IRQHandlerSV, %function
RTC_Alarm_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl RTC_Alarm_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  RTC_Alarm_IRQHandlerSV, .-RTC_Alarm_IRQHandlerSV


.section  .text.USBWakeUp_IRQHandlerSV
.weak  USBWakeUp_IRQHandlerSV
.type USBWakeUp_IRQHandlerSV, %function
USBWakeUp_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl USBWakeUp_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  USBWakeUp_IRQHandlerSV, .-USBWakeUp_IRQHandlerSV


.section  .text.TIM8_BRK_IRQHandlerSV
.weak  TIM8_BRK_IRQHandlerSV
.type TIM8_BRK_IRQHandlerSV, %function
TIM8_BRK_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl TIM8_BRK_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  TIM8_BRK_IRQHandlerSV, .-TIM8_BRK_IRQHandlerSV


.section  .text.TIM8_UP_IRQHandlerSV
.weak  TIM8_UP_IRQHandlerSV
.type TIM8_UP_IRQHandlerSV, %function
TIM8_UP_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl TIM8_UP_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  TIM8_UP_IRQHandlerSV, .-TIM8_UP_IRQHandlerSV


.section  .text.TIM8_TRG_COM_IRQHandlerSV
.weak  TIM8_TRG_COM_IRQHandlerSV
.type TIM8_TRG_COM_IRQHandlerSV, %function
TIM8_TRG_COM_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl TIM8_TRG_COM_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  TIM8_TRG_COM_IRQHandlerSV, .-TIM8_TRG_COM_IRQHandlerSV


.section  .text.TIM8_CC_IRQHandlerSV
.weak  TIM8_CC_IRQHandlerSV
.type TIM8_CC_IRQHandlerSV, %function
TIM8_CC_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl TIM8_CC_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  TIM8_CC_IRQHandlerSV, .-TIM8_CC_IRQHandlerSV


.section  .text.ADC3_IRQHandlerSV
.weak  ADC3_IRQHandlerSV
.type ADC3_IRQHandlerSV, %function
ADC3_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl ADC3_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  ADC3_IRQHandlerSV, .-ADC3_IRQHandlerSV


.section  .text.SPI3_IRQHandlerSV
.weak  SPI3_IRQHandlerSV
.type SPI3_IRQHandlerSV, %function
SPI3_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl SPI3_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  SPI3_IRQHandlerSV, .-SPI3_IRQHandlerSV


.section  .text.UART4_IRQHandlerSV
.weak  UART4_IRQHandlerSV
.type UART4_IRQHandlerSV, %function
UART4_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl UART4_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  UART4_IRQHandlerSV, .-UART4_IRQHandlerSV


.section  .text.UART5_IRQHandlerSV
.weak  UART5_IRQHandlerSV
.type UART5_IRQHandlerSV, %function
UART5_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl UART5_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  UART5_IRQHandlerSV, .-UART5_IRQHandlerSV


.section  .text.TIM6_DAC_IRQHandlerSV
.weak  TIM6_DAC_IRQHandlerSV
.type TIM6_DAC_IRQHandlerSV, %function
TIM6_DAC_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl TIM6_DAC_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  TIM6_DAC_IRQHandlerSV, .-TIM6_DAC_IRQHandlerSV


.section  .text.TIM7_IRQHandlerSV
.weak  TIM7_IRQHandlerSV
.type TIM7_IRQHandlerSV, %function
TIM7_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl TIM7_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  TIM7_IRQHandlerSV, .-TIM7_IRQHandlerSV


.section  .text.DMA2_Channel1_IRQHandlerSV
.weak  DMA2_Channel1_IRQHandlerSV
.type DMA2_Channel1_IRQHandlerSV, %function
DMA2_Channel1_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl DMA2_Channel1_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  DMA2_Channel1_IRQHandlerSV, .-DMA2_Channel1_IRQHandlerSV


.section  .text.DMA2_Channel2_IRQHandlerSV
.weak  DMA2_Channel2_IRQHandlerSV
.type DMA2_Channel2_IRQHandlerSV, %function
DMA2_Channel2_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl DMA2_Channel2_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  DMA2_Channel2_IRQHandlerSV, .-DMA2_Channel2_IRQHandlerSV


.section  .text.DMA2_Channel3_IRQHandlerSV
.weak  DMA2_Channel3_IRQHandlerSV
.type DMA2_Channel3_IRQHandlerSV, %function
DMA2_Channel3_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl DMA2_Channel3_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  DMA2_Channel3_IRQHandlerSV, .-DMA2_Channel3_IRQHandlerSV


.section  .text.DMA2_Channel4_IRQHandlerSV
.weak  DMA2_Channel4_IRQHandlerSV
.type DMA2_Channel4_IRQHandlerSV, %function
DMA2_Channel4_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl DMA2_Channel4_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  DMA2_Channel4_IRQHandlerSV, .-DMA2_Channel4_IRQHandlerSV


.section  .text.DMA2_Channel5_IRQHandlerSV
.weak  DMA2_Channel5_IRQHandlerSV
.type DMA2_Channel5_IRQHandlerSV, %function
DMA2_Channel5_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl DMA2_Channel5_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  DMA2_Channel5_IRQHandlerSV, .-DMA2_Channel5_IRQHandlerSV


.section  .text.ADC4_IRQHandlerSV
.weak  ADC4_IRQHandlerSV
.type ADC4_IRQHandlerSV, %function
ADC4_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl ADC4_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  ADC4_IRQHandlerSV, .-ADC4_IRQHandlerSV


.section  .text.COMP1_2_3_IRQHandlerSV
.weak  COMP1_2_3_IRQHandlerSV
.type COMP1_2_3_IRQHandlerSV, %function
COMP1_2_3_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl COMP1_2_3_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  COMP1_2_3_IRQHandlerSV, .-COMP1_2_3_IRQHandlerSV


.section  .text.COMP4_5_6_IRQHandlerSV
.weak  COMP4_5_6_IRQHandlerSV
.type COMP4_5_6_IRQHandlerSV, %function
COMP4_5_6_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl COMP4_5_6_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  COMP4_5_6_IRQHandlerSV, .-COMP4_5_6_IRQHandlerSV


.section  .text.COMP7_IRQHandlerSV
.weak  COMP7_IRQHandlerSV
.type COMP7_IRQHandlerSV, %function
COMP7_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl COMP7_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  COMP7_IRQHandlerSV, .-COMP7_IRQHandlerSV


.section  .text.USB_HP_IRQHandlerSV
.weak  USB_HP_IRQHandlerSV
.type USB_HP_IRQHandlerSV, %function
USB_HP_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl USB_HP_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  USB_HP_IRQHandlerSV, .-USB_HP_IRQHandlerSV


.section  .text.USB_LP_IRQHandlerSV
.weak  USB_LP_IRQHandlerSV
.type USB_LP_IRQHandlerSV, %function
USB_LP_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl USB_LP_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  USB_LP_IRQHandlerSV, .-USB_LP_IRQHandlerSV


.section  .text.USBWakeUp_RMP_IRQHandlerSV
.weak  USBWakeUp_RMP_IRQHandlerSV
.type USBWakeUp_RMP_IRQHandlerSV, %function
USBWakeUp_RMP_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl USBWakeUp_RMP_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  USBWakeUp_RMP_IRQHandlerSV, .-USBWakeUp_RMP_IRQHandlerSV


.section  .text.FPU_IRQHandlerSV
.weak  FPU_IRQHandlerSV
.type FPU_IRQHandlerSV, %function
FPU_IRQHandlerSV:
	push    {r7, lr}
	sub     sp, #8
	add     r7, sp, #0
	bl SEGGER_SYSVIEW_RecordEnterISR
	bl FPU_IRQHandler
	bl SEGGER_SYSVIEW_RecordExitISR
	adds    r7, #8
	mov     sp, r7
	pop     {r7, pc}
.size  FPU_IRQHandlerSV, .-FPU_IRQHandlerSV

#endif /* SYSVIEW */

/******************************************************************************
*
* The minimal vector table for a Cortex M4. Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
* 
*******************************************************************************/
   .section  .isr_vector,"a",%progbits
  .type  g_pfnVectors, %object
  .size  g_pfnVectors, .-g_pfnVectors

#ifdef SYSVIEW

g_pfnVectors:
	.word	_estack
	.word	Reset_Handler
	.word	NMI_Handler
	.word	HardFault_Handler
	.word	MemManage_Handler
	.word	BusFault_Handler
	.word	UsageFault_Handler
	.word	0
	.word	0
	.word	0
	.word	0
	.word	SVC_Handler
	.word	DebugMon_Handler
	.word	0
	.word	PendSV_Handler
	.word	SysTick_Handler
	.word	WWDG_IRQHandlerSV
	.word	PVD_IRQHandlerSV
	.word	TAMPER_STAMP_IRQHandlerSV
	.word	RTC_WKUP_IRQHandlerSV
	.word	FLASH_IRQHandlerSV
	.word	RCC_IRQHandlerSV
	.word	EXTI0_IRQHandlerSV
	.word	EXTI1_IRQHandlerSV
	.word	EXTI2_TS_IRQHandlerSV
	.word	EXTI3_IRQHandlerSV
	.word	EXTI4_IRQHandlerSV
	.word	DMA1_Channel1_IRQHandlerSV
	.word	DMA1_Channel2_IRQHandlerSV
	.word	DMA1_Channel3_IRQHandlerSV
	.word	DMA1_Channel4_IRQHandlerSV
	.word	DMA1_Channel5_IRQHandlerSV
	.word	DMA1_Channel6_IRQHandlerSV
	.word	DMA1_Channel7_IRQHandlerSV
	.word	ADC1_2_IRQHandlerSV
	.word	USB_HP_CAN1_TX_IRQHandlerSV
	.word	USB_LP_CAN1_RX0_IRQHandlerSV
	.word	CAN1_RX1_IRQHandlerSV
	.word	CAN1_SCE_IRQHandlerSV
	.word	EXTI9_5_IRQHandlerSV
	.word	TIM1_BRK_TIM15_IRQHandlerSV
	.word	TIM1_UP_TIM16_IRQHandlerSV
	.word	TIM1_TRG_COM_TIM17_IRQHandlerSV
	.word	TIM1_CC_IRQHandlerSV
	.word	TIM2_IRQHandlerSV
	.word	TIM3_IRQHandlerSV
	.word	TIM4_IRQHandlerSV
	.word	I2C1_EV_IRQHandlerSV
	.word	I2C1_ER_IRQHandlerSV
	.word	I2C2_EV_IRQHandlerSV
	.word	I2C2_ER_IRQHandlerSV
	.word	SPI1_IRQHandlerSV
	.word	SPI2_IRQHandlerSV
	.word	USART1_IRQHandlerSV
	.word	USART2_IRQHandlerSV
	.word	USART3_IRQHandlerSV
	.word	EXTI15_10_IRQHandlerSV
	.word	RTC_Alarm_IRQHandlerSV
	.word	USBWakeUp_IRQHandlerSV
	.word	TIM8_BRK_IRQHandlerSV
	.word	TIM8_UP_IRQHandlerSV
	.word	TIM8_TRG_COM_IRQHandlerSV
	.word	TIM8_CC_IRQHandlerSV
	.word	ADC3_IRQHandlerSV
	.word	0
	.word	0
	.word	0
	.word	SPI3_IRQHandlerSV
	.word	UART4_IRQHandlerSV
	.word	UART5_IRQHandlerSV
	.word	TIM6_DAC_IRQHandlerSV
	.word	TIM7_IRQHandlerSV
	.word	DMA2_Channel1_IRQHandlerSV
	.word	DMA2_Channel2_IRQHandlerSV
	.word	DMA2_Channel3_IRQHandlerSV
	.word	DMA2_Channel4_IRQHandlerSV
	.word	DMA2_Channel5_IRQHandlerSV
	.word	ADC4_IRQHandlerSV
	.word	0
	.word	0
	.word	COMP1_2_3_IRQHandlerSV
	.word	COMP4_5_6_IRQHandlerSV
	.word	COMP7_IRQHandlerSV
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	USB_HP_IRQHandlerSV
	.word	USB_LP_IRQHandlerSV
	.word	USBWakeUp_RMP_IRQHandlerSV
	.word	0
	.word	0
	.word	0
	.word	0
	.word	FPU_IRQHandlerSV

#else

g_pfnVectors:
	.word	_estack
	.word	Reset_Handler
	.word	NMI_Handler
	.word	HardFault_Handler
	.word	MemManage_Handler
	.word	BusFault_Handler
	.word	UsageFault_Handler
	.word	0
	.word	0
	.word	0
	.word	0
	.word	SVC_Handler
	.word	DebugMon_Handler
	.word	0
	.word	PendSV_Handler
	.word	SysTick_Handler
	.word	WWDG_IRQHandler
	.word	PVD_IRQHandler
	.word	TAMPER_STAMP_IRQHandler
	.word	RTC_WKUP_IRQHandler
	.word	FLASH_IRQHandler
	.word	RCC_IRQHandler
	.word	EXTI0_IRQHandler
	.word	EXTI1_IRQHandler
	.word	EXTI2_TS_IRQHandler
	.word	EXTI3_IRQHandler
	.word	EXTI4_IRQHandler
	.word	DMA1_Channel1_IRQHandler
	.word	DMA1_Channel2_IRQHandler
	.word	DMA1_Channel3_IRQHandler
	.word	DMA1_Channel4_IRQHandler
	.word	DMA1_Channel5_IRQHandler
	.word	DMA1_Channel6_IRQHandler
	.word	DMA1_Channel7_IRQHandler
	.word	ADC1_2_IRQHandler
	.word	USB_HP_CAN1_TX_IRQHandler
	.word	USB_LP_CAN1_RX0_IRQHandler
	.word	CAN1_RX1_IRQHandler
	.word	CAN1_SCE_IRQHandler
	.word	EXTI9_5_IRQHandler
	.word	TIM1_BRK_TIM15_IRQHandler
	.word	TIM1_UP_TIM16_IRQHandler
	.word	TIM1_TRG_COM_TIM17_IRQHandler
	.word	TIM1_CC_IRQHandler
	.word	TIM2_IRQHandler
	.word	TIM3_IRQHandler
	.word	TIM4_IRQHandler
	.word	I2C1_EV_IRQHandler
	.word	I2C1_ER_IRQHandler
	.word	I2C2_EV_IRQHandler
	.word	I2C2_ER_IRQHandler
	.word	SPI1_IRQHandler
	.word	SPI2_IRQHandler
	.word	USART1_IRQHandler
	.word	USART2_IRQHandler
	.word	USART3_IRQHandler
	.word	EXTI15_10_IRQHandler
	.word	RTC_Alarm_IRQHandler
	.word	USBWakeUp_IRQHandler
	.word	TIM8_BRK_IRQHandler
	.word	TIM8_UP_IRQHandler
	.word	TIM8_TRG_COM_IRQHandler
	.word	TIM8_CC_IRQHandler
	.word	ADC3_IRQHandler
	.word	0
	.word	0
	.word	0
	.word	SPI3_IRQHandler
	.word	UART4_IRQHandler
	.word	UART5_IRQHandler
	.word	TIM6_DAC_IRQHandler
	.word	TIM7_IRQHandler
	.word	DMA2_Channel1_IRQHandler
	.word	DMA2_Channel2_IRQHandler
	.word	DMA2_Channel3_IRQHandler
	.word	DMA2_Channel4_IRQHandler
	.word	DMA2_Channel5_IRQHandler
	.word	ADC4_IRQHandler
	.word	0
	.word	0
	.word	COMP1_2_3_IRQHandler
	.word	COMP4_5_6_IRQHandler
	.word	COMP7_IRQHandler
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	USB_HP_IRQHandler
	.word	USB_LP_IRQHandler
	.word	USBWakeUp_RMP_IRQHandler
	.word	0
	.word	0
	.word	0
	.word	0
	.word	FPU_IRQHandler

#endif

/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler.
* As they are weak aliases, any function with the same name will override
* this definition.
*
*******************************************************************************/

  	.weak	NMI_Handler
	.thumb_set NMI_Handler,Default_Handler

  	.weak	HardFault_Handler
	.thumb_set HardFault_Handler,Default_Handler

  	.weak	MemManage_Handler
	.thumb_set MemManage_Handler,Default_Handler

  	.weak	BusFault_Handler
	.thumb_set BusFault_Handler,Default_Handler

	.weak	UsageFault_Handler
	.thumb_set UsageFault_Handler,Default_Handler

	.weak	SVC_Handler
	.thumb_set SVC_Handler,Default_Handler

	.weak	DebugMon_Handler
	.thumb_set DebugMon_Handler,Default_Handler

	.weak	PendSV_Handler
	.thumb_set PendSV_Handler,Default_Handler

	.weak	SysTick_Handler
	.thumb_set SysTick_Handler,Default_Handler

	.weak	WWDG_IRQHandler
	.thumb_set WWDG_IRQHandler,Default_Handler

	.weak	PVD_IRQHandler
	.thumb_set PVD_IRQHandler,Default_Handler

	.weak	TAMPER_STAMP_IRQHandler
	.thumb_set TAMPER_STAMP_IRQHandler,Default_Handler

	.weak	RTC_WKUP_IRQHandler
	.thumb_set RTC_WKUP_IRQHandler,Default_Handler

	.weak	FLASH_IRQHandler
	.thumb_set FLASH_IRQHandler,Default_Handler

	.weak	RCC_IRQHandler
	.thumb_set RCC_IRQHandler,Default_Handler

	.weak	EXTI0_IRQHandler
	.thumb_set EXTI0_IRQHandler,Default_Handler

	.weak	EXTI1_IRQHandler
	.thumb_set EXTI1_IRQHandler,Default_Handler

	.weak	EXTI2_TS_IRQHandler
	.thumb_set EXTI2_TS_IRQHandler,Default_Handler

	.weak	EXTI3_IRQHandler
	.thumb_set EXTI3_IRQHandler,Default_Handler

	.weak	EXTI4_IRQHandler
	.thumb_set EXTI4_IRQHandler,Default_Handler

	.weak	DMA1_Channel1_IRQHandler
	.thumb_set DMA1_Channel1_IRQHandler,Default_Handler

	.weak	DMA1_Channel2_IRQHandler
	.thumb_set DMA1_Channel2_IRQHandler,Default_Handler

	.weak	DMA1_Channel3_IRQHandler
	.thumb_set DMA1_Channel3_IRQHandler,Default_Handler

	.weak	DMA1_Channel4_IRQHandler
	.thumb_set DMA1_Channel4_IRQHandler,Default_Handler

	.weak	DMA1_Channel5_IRQHandler
	.thumb_set DMA1_Channel5_IRQHandler,Default_Handler

	.weak	DMA1_Channel6_IRQHandler
	.thumb_set DMA1_Channel6_IRQHandler,Default_Handler

	.weak	DMA1_Channel7_IRQHandler
	.thumb_set DMA1_Channel7_IRQHandler,Default_Handler

	.weak	ADC1_2_IRQHandler
	.thumb_set ADC1_2_IRQHandler,Default_Handler

	.weak	USB_HP_CAN1_TX_IRQHandler
	.thumb_set USB_HP_CAN1_TX_IRQHandler,Default_Handler

	.weak	USB_LP_CAN1_RX0_IRQHandler
	.thumb_set USB_LP_CAN1_RX0_IRQHandler,Default_Handler

	.weak	CAN1_RX1_IRQHandler
	.thumb_set CAN1_RX1_IRQHandler,Default_Handler

	.weak	CAN1_SCE_IRQHandler
	.thumb_set CAN1_SCE_IRQHandler,Default_Handler

	.weak	EXTI9_5_IRQHandler
	.thumb_set EXTI9_5_IRQHandler,Default_Handler

	.weak	TIM1_BRK_TIM15_IRQHandler
	.thumb_set TIM1_BRK_TIM15_IRQHandler,Default_Handler

	.weak	TIM1_UP_TIM16_IRQHandler
	.thumb_set TIM1_UP_TIM16_IRQHandler,Default_Handler

	.weak	TIM1_TRG_COM_TIM17_IRQHandler
	.thumb_set TIM1_TRG_COM_TIM17_IRQHandler,Default_Handler

	.weak	TIM1_CC_IRQHandler
	.thumb_set TIM1_CC_IRQHandler,Default_Handler

	.weak	TIM2_IRQHandler
	.thumb_set TIM2_IRQHandler,Default_Handler

	.weak	TIM3_IRQHandler
	.thumb_set TIM3_IRQHandler,Default_Handler

	.weak	TIM4_IRQHandler
	.thumb_set TIM4_IRQHandler,Default_Handler

	.weak	I2C1_EV_IRQHandler
	.thumb_set I2C1_EV_IRQHandler,Default_Handler

	.weak	I2C1_ER_IRQHandler
	.thumb_set I2C1_ER_IRQHandler,Default_Handler

	.weak	I2C2_EV_IRQHandler
	.thumb_set I2C2_EV_IRQHandler,Default_Handler

	.weak	I2C2_ER_IRQHandler
	.thumb_set I2C2_ER_IRQHandler,Default_Handler

	.weak	SPI1_IRQHandler
	.thumb_set SPI1_IRQHandler,Default_Handler

	.weak	SPI2_IRQHandler
	.thumb_set SPI2_IRQHandler,Default_Handler

	.weak	USART1_IRQHandler
	.thumb_set USART1_IRQHandler,Default_Handler

	.weak	USART2_IRQHandler
	.thumb_set USART2_IRQHandler,Default_Handler

	.weak	USART3_IRQHandler
	.thumb_set USART3_IRQHandler,Default_Handler

	.weak	EXTI15_10_IRQHandler
	.thumb_set EXTI15_10_IRQHandler,Default_Handler

	.weak	RTC_Alarm_IRQHandler
	.thumb_set RTC_Alarm_IRQHandler,Default_Handler

	.weak	USBWakeUp_IRQHandler
	.thumb_set USBWakeUp_IRQHandler,Default_Handler

	.weak	TIM8_BRK_IRQHandler
	.thumb_set TIM8_BRK_IRQHandler,Default_Handler

	.weak	TIM8_UP_IRQHandler
	.thumb_set TIM8_UP_IRQHandler,Default_Handler

	.weak	TIM8_TRG_COM_IRQHandler
	.thumb_set TIM8_TRG_COM_IRQHandler,Default_Handler

	.weak	TIM8_CC_IRQHandler
	.thumb_set TIM8_CC_IRQHandler,Default_Handler

	.weak	ADC3_IRQHandler
	.thumb_set ADC3_IRQHandler,Default_Handler

	.weak	SPI3_IRQHandler
	.thumb_set SPI3_IRQHandler,Default_Handler

	.weak	UART4_IRQHandler
	.thumb_set UART4_IRQHandler,Default_Handler

	.weak	UART5_IRQHandler
	.thumb_set UART5_IRQHandler,Default_Handler

	.weak	TIM6_DAC_IRQHandler
	.thumb_set TIM6_DAC_IRQHandler,Default_Handler

	.weak	TIM7_IRQHandler
	.thumb_set TIM7_IRQHandler,Default_Handler

	.weak	DMA2_Channel1_IRQHandler
	.thumb_set DMA2_Channel1_IRQHandler,Default_Handler

	.weak	DMA2_Channel2_IRQHandler
	.thumb_set DMA2_Channel2_IRQHandler,Default_Handler

	.weak	DMA2_Channel3_IRQHandler
	.thumb_set DMA2_Channel3_IRQHandler,Default_Handler

	.weak	DMA2_Channel4_IRQHandler
	.thumb_set DMA2_Channel4_IRQHandler,Default_Handler

	.weak	DMA2_Channel5_IRQHandler
	.thumb_set DMA2_Channel5_IRQHandler,Default_Handler

	.weak	ADC4_IRQHandler
	.thumb_set ADC4_IRQHandler,Default_Handler	
	
	.weak	COMP1_2_3_IRQHandler
	.thumb_set COMP1_2_3_IRQHandler,Default_Handler
	
	.weak	COMP4_5_6_IRQHandler
	.thumb_set COMP4_5_6_IRQHandler,Default_Handler
	
	.weak	COMP7_IRQHandler
	.thumb_set COMP7_IRQHandler,Default_Handler	
	
	.weak	USB_HP_IRQHandler
	.thumb_set USB_HP_IRQHandler,Default_Handler
	
	.weak	USB_LP_IRQHandler
	.thumb_set USB_LP_IRQHandler,Default_Handler
	
	.weak	USBWakeUp_RMP_IRQHandler
	.thumb_set USBWakeUp_RMP_IRQHandler,Default_Handler
	
	.weak	FPU_IRQHandler
	.thumb_set FPU_IRQHandler,Default_Handler
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
