/**
  ******************************************************************************
  * @file      startup_stm32f40xx.s
  * @author    MCD Application Team
  * @version   V1.8.0
  * @date      09-November-2016
  * @brief     STM32F40xxxx Devices vector table for RIDE7 toolchain.
  *            This module performs:
  *                - Set the initial SP
  *                - Set the initial PC == Reset_Handler,
  *                - Set the vector table entries with the exceptions ISR address
  *                - Configure the clock system and the external SRAM mounted on 
  *                  STM324xG-EVAL board to be used as data memory (optional, 
  *                  to be enabled by user)
  *                - Branches to main in the C library (which eventually
  *                  calls main()).
  *            After Reset the Cortex-M4 processor is in Thread mode,
  *            priority is Privileged, and the Stack is set to Main.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2016 STMicroelectronics</center></h2>
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

/* Call the clock system intitialization function.*/
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

/**
 * @brief  This are special SYSVIEW interrupthandler to enable tracing of interruptcalls.
 * @param  None
 * @retval None
*/
#ifdef SYSVIEW

.section  .text.WWDG_IRQHandlerSV
.weak WWDG_IRQHandlerSV
.type WWDG_IRQHandlerSV, %function
WWDG_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl WWDG_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size WWDG_IRQHandlerSV, .-WWDG_IRQHandlerSV

.section  .text.PVD_IRQHandlerSV
.weak PVD_IRQHandlerSV
.type PVD_IRQHandlerSV, %function
PVD_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl PVD_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size PVD_IRQHandlerSV, .-PVD_IRQHandlerSV

.section  .text.TAMP_STAMP_IRQHandlerSV
.weak TAMP_STAMP_IRQHandlerSV
.type TAMP_STAMP_IRQHandlerSV, %function
TAMP_STAMP_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl TAMP_STAMP_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size TAMP_STAMP_IRQHandlerSV, .-TAMP_STAMP_IRQHandlerSV

.section  .text.RTC_WKUP_IRQHandlerSV
.weak RTC_WKUP_IRQHandlerSV
.type RTC_WKUP_IRQHandlerSV, %function
RTC_WKUP_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl RTC_WKUP_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size RTC_WKUP_IRQHandlerSV, .-RTC_WKUP_IRQHandlerSV

.section  .text.FLASH_IRQHandlerSV
.weak FLASH_IRQHandlerSV
.type FLASH_IRQHandlerSV, %function
FLASH_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl FLASH_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size FLASH_IRQHandlerSV, .-FLASH_IRQHandlerSV

.section  .text.RCC_IRQHandlerSV
.weak RCC_IRQHandlerSV
.type RCC_IRQHandlerSV, %function
RCC_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl RCC_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size RCC_IRQHandlerSV, .-RCC_IRQHandlerSV

.section  .text.EXTI0_IRQHandlerSV
.weak EXTI0_IRQHandlerSV
.type EXTI0_IRQHandlerSV, %function
EXTI0_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl EXTI0_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size EXTI0_IRQHandlerSV, .-EXTI0_IRQHandlerSV

.section  .text.EXTI1_IRQHandlerSV
.weak EXTI1_IRQHandlerSV
.type EXTI1_IRQHandlerSV, %function
EXTI1_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl EXTI1_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size EXTI1_IRQHandlerSV, .-EXTI1_IRQHandlerSV

.section  .text.EXTI2_IRQHandlerSV
.weak EXTI2_IRQHandlerSV
.type EXTI2_IRQHandlerSV, %function
EXTI2_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl EXTI2_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size EXTI2_IRQHandlerSV, .-EXTI2_IRQHandlerSV

.section  .text.EXTI3_IRQHandlerSV
.weak EXTI3_IRQHandlerSV
.type EXTI3_IRQHandlerSV, %function
EXTI3_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl EXTI3_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size EXTI3_IRQHandlerSV, .-EXTI3_IRQHandlerSV

.section  .text.EXTI4_IRQHandlerSV
.weak EXTI4_IRQHandlerSV
.type EXTI4_IRQHandlerSV, %function
EXTI4_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl EXTI4_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size EXTI4_IRQHandlerSV, .-EXTI4_IRQHandlerSV

.section  .text.DMA1_Stream0_IRQHandlerSV
.weak DMA1_Stream0_IRQHandlerSV
.type DMA1_Stream0_IRQHandlerSV, %function
DMA1_Stream0_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl DMA1_Stream0_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size DMA1_Stream0_IRQHandlerSV, .-DMA1_Stream0_IRQHandlerSV

.section  .text.DMA1_Stream1_IRQHandlerSV
.weak DMA1_Stream1_IRQHandlerSV
.type DMA1_Stream1_IRQHandlerSV, %function
DMA1_Stream1_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl DMA1_Stream1_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size DMA1_Stream1_IRQHandlerSV, .-DMA1_Stream1_IRQHandlerSV

.section  .text.DMA1_Stream2_IRQHandlerSV
.weak DMA1_Stream2_IRQHandlerSV
.type DMA1_Stream2_IRQHandlerSV, %function
DMA1_Stream2_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl DMA1_Stream2_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size DMA1_Stream2_IRQHandlerSV, .-DMA1_Stream2_IRQHandlerSV

.section  .text.DMA1_Stream3_IRQHandlerSV
.weak DMA1_Stream3_IRQHandlerSV
.type DMA1_Stream3_IRQHandlerSV, %function
DMA1_Stream3_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl DMA1_Stream3_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size DMA1_Stream3_IRQHandlerSV, .-DMA1_Stream3_IRQHandlerSV

.section  .text.DMA1_Stream4_IRQHandlerSV
.weak DMA1_Stream4_IRQHandlerSV
.type DMA1_Stream4_IRQHandlerSV, %function
DMA1_Stream4_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl DMA1_Stream4_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size DMA1_Stream4_IRQHandlerSV, .-DMA1_Stream4_IRQHandlerSV

.section  .text.DMA1_Stream5_IRQHandlerSV
.weak DMA1_Stream5_IRQHandlerSV
.type DMA1_Stream5_IRQHandlerSV, %function
DMA1_Stream5_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl DMA1_Stream5_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size DMA1_Stream5_IRQHandlerSV, .-DMA1_Stream5_IRQHandlerSV

.section  .text.DMA1_Stream6_IRQHandlerSV
.weak DMA1_Stream6_IRQHandlerSV
.type DMA1_Stream6_IRQHandlerSV, %function
DMA1_Stream6_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl DMA1_Stream6_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size DMA1_Stream6_IRQHandlerSV, .-DMA1_Stream6_IRQHandlerSV

.section  .text.ADC_IRQHandlerSV
.weak ADC_IRQHandlerSV
.type ADC_IRQHandlerSV, %function
ADC_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl ADC_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size ADC_IRQHandlerSV, .-ADC_IRQHandlerSV

.section  .text.CAN1_TX_IRQHandlerSV
.weak CAN1_TX_IRQHandlerSV
.type CAN1_TX_IRQHandlerSV, %function
CAN1_TX_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl CAN1_TX_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size CAN1_TX_IRQHandlerSV, .-CAN1_TX_IRQHandlerSV

.section  .text.CAN1_RX0_IRQHandlerSV
.weak CAN1_RX0_IRQHandlerSV
.type CAN1_RX0_IRQHandlerSV, %function
CAN1_RX0_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl CAN1_RX0_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size CAN1_RX0_IRQHandlerSV, .-CAN1_RX0_IRQHandlerSV

.section  .text.CAN1_RX1_IRQHandlerSV
.weak CAN1_RX1_IRQHandlerSV
.type CAN1_RX1_IRQHandlerSV, %function
CAN1_RX1_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl CAN1_RX1_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size CAN1_RX1_IRQHandlerSV, .-CAN1_RX1_IRQHandlerSV

.section  .text.CAN1_SCE_IRQHandlerSV
.weak CAN1_SCE_IRQHandlerSV
.type CAN1_SCE_IRQHandlerSV, %function
CAN1_SCE_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl CAN1_SCE_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size CAN1_SCE_IRQHandlerSV, .-CAN1_SCE_IRQHandlerSV

.section  .text.EXTI9_5_IRQHandlerSV
.weak EXTI9_5_IRQHandlerSV
.type EXTI9_5_IRQHandlerSV, %function
EXTI9_5_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl EXTI9_5_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size EXTI9_5_IRQHandlerSV, .-EXTI9_5_IRQHandlerSV

.section  .text.TIM1_BRK_TIM9_IRQHandlerSV
.weak TIM1_BRK_TIM9_IRQHandlerSV
.type TIM1_BRK_TIM9_IRQHandlerSV, %function
TIM1_BRK_TIM9_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl TIM1_BRK_TIM9_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size TIM1_BRK_TIM9_IRQHandlerSV, .-TIM1_BRK_TIM9_IRQHandlerSV

.section  .text.TIM1_UP_TIM10_IRQHandlerSV
.weak TIM1_UP_TIM10_IRQHandlerSV
.type TIM1_UP_TIM10_IRQHandlerSV, %function
TIM1_UP_TIM10_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl TIM1_UP_TIM10_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size TIM1_UP_TIM10_IRQHandlerSV, .-TIM1_UP_TIM10_IRQHandlerSV

.section  .text.TIM1_TRG_COM_TIM11_IRQHandlerSV
.weak TIM1_TRG_COM_TIM11_IRQHandlerSV
.type TIM1_TRG_COM_TIM11_IRQHandlerSV, %function
TIM1_TRG_COM_TIM11_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl TIM1_TRG_COM_TIM11_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size TIM1_TRG_COM_TIM11_IRQHandlerSV, .-TIM1_TRG_COM_TIM11_IRQHandlerSV

.section  .text.TIM1_CC_IRQHandlerSV
.weak TIM1_CC_IRQHandlerSV
.type TIM1_CC_IRQHandlerSV, %function
TIM1_CC_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl TIM1_CC_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size TIM1_CC_IRQHandlerSV, .-TIM1_CC_IRQHandlerSV

.section  .text.TIM2_IRQHandlerSV
.weak TIM2_IRQHandlerSV
.type TIM2_IRQHandlerSV, %function
TIM2_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl TIM2_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size TIM2_IRQHandlerSV, .-TIM2_IRQHandlerSV

.section  .text.TIM3_IRQHandlerSV
.weak TIM3_IRQHandlerSV
.type TIM3_IRQHandlerSV, %function
TIM3_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl TIM3_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size TIM3_IRQHandlerSV, .-TIM3_IRQHandlerSV

.section  .text.TIM4_IRQHandlerSV
.weak TIM4_IRQHandlerSV
.type TIM4_IRQHandlerSV, %function
TIM4_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl TIM4_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size TIM4_IRQHandlerSV, .-TIM4_IRQHandlerSV

.section  .text.I2C1_EV_IRQHandlerSV
.weak I2C1_EV_IRQHandlerSV
.type I2C1_EV_IRQHandlerSV, %function
I2C1_EV_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl I2C1_EV_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size I2C1_EV_IRQHandlerSV, .-I2C1_EV_IRQHandlerSV

.section  .text.I2C1_ER_IRQHandlerSV
.weak I2C1_ER_IRQHandlerSV
.type I2C1_ER_IRQHandlerSV, %function
I2C1_ER_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl I2C1_ER_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size I2C1_ER_IRQHandlerSV, .-I2C1_ER_IRQHandlerSV

.section  .text.I2C2_EV_IRQHandlerSV
.weak I2C2_EV_IRQHandlerSV
.type I2C2_EV_IRQHandlerSV, %function
I2C2_EV_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl I2C2_EV_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size I2C2_EV_IRQHandlerSV, .-I2C2_EV_IRQHandlerSV

.section  .text.I2C2_ER_IRQHandlerSV
.weak I2C2_ER_IRQHandlerSV
.type I2C2_ER_IRQHandlerSV, %function
I2C2_ER_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl I2C2_ER_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size I2C2_ER_IRQHandlerSV, .-I2C2_ER_IRQHandlerSV

.section  .text.SPI1_IRQHandlerSV
.weak SPI1_IRQHandlerSV
.type SPI1_IRQHandlerSV, %function
SPI1_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl SPI1_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size SPI1_IRQHandlerSV, .-SPI1_IRQHandlerSV

.section  .text.SPI2_IRQHandlerSV
.weak SPI2_IRQHandlerSV
.type SPI2_IRQHandlerSV, %function
SPI2_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl SPI2_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size SPI2_IRQHandlerSV, .-SPI2_IRQHandlerSV

.section  .text.USART1_IRQHandlerSV
.weak USART1_IRQHandlerSV
.type USART1_IRQHandlerSV, %function
USART1_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl USART1_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size USART1_IRQHandlerSV, .-USART1_IRQHandlerSV

.section  .text.USART2_IRQHandlerSV
.weak USART2_IRQHandlerSV
.type USART2_IRQHandlerSV, %function
USART2_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl USART2_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size USART2_IRQHandlerSV, .-USART2_IRQHandlerSV

.section  .text.USART3_IRQHandlerSV
.weak USART3_IRQHandlerSV
.type USART3_IRQHandlerSV, %function
USART3_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl USART3_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size USART3_IRQHandlerSV, .-USART3_IRQHandlerSV

.section  .text.EXTI15_10_IRQHandlerSV
.weak EXTI15_10_IRQHandlerSV
.type EXTI15_10_IRQHandlerSV, %function
EXTI15_10_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl EXTI15_10_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size EXTI15_10_IRQHandlerSV, .-EXTI15_10_IRQHandlerSV

.section  .text.RTC_Alarm_IRQHandlerSV
.weak RTC_Alarm_IRQHandlerSV
.type RTC_Alarm_IRQHandlerSV, %function
RTC_Alarm_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl RTC_Alarm_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size RTC_Alarm_IRQHandlerSV, .-RTC_Alarm_IRQHandlerSV

.section  .text.OTG_FS_WKUP_IRQHandlerSV
.weak OTG_FS_WKUP_IRQHandlerSV
.type OTG_FS_WKUP_IRQHandlerSV, %function
OTG_FS_WKUP_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl OTG_FS_WKUP_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size OTG_FS_WKUP_IRQHandlerSV, .-OTG_FS_WKUP_IRQHandlerSV

.section  .text.TIM8_BRK_TIM12_IRQHandlerSV
.weak TIM8_BRK_TIM12_IRQHandlerSV
.type TIM8_BRK_TIM12_IRQHandlerSV, %function
TIM8_BRK_TIM12_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl TIM8_BRK_TIM12_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size TIM8_BRK_TIM12_IRQHandlerSV, .-TIM8_BRK_TIM12_IRQHandlerSV

.section  .text.TIM8_UP_TIM13_IRQHandlerSV
.weak TIM8_UP_TIM13_IRQHandlerSV
.type TIM8_UP_TIM13_IRQHandlerSV, %function
TIM8_UP_TIM13_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl TIM8_UP_TIM13_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size TIM8_UP_TIM13_IRQHandlerSV, .-TIM8_UP_TIM13_IRQHandlerSV

.section  .text.TIM8_TRG_COM_TIM14_IRQHandlerSV
.weak TIM8_TRG_COM_TIM14_IRQHandlerSV
.type TIM8_TRG_COM_TIM14_IRQHandlerSV, %function
TIM8_TRG_COM_TIM14_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl TIM8_TRG_COM_TIM14_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size TIM8_TRG_COM_TIM14_IRQHandlerSV, .-TIM8_TRG_COM_TIM14_IRQHandlerSV

.section  .text.TIM8_CC_IRQHandlerSV
.weak TIM8_CC_IRQHandlerSV
.type TIM8_CC_IRQHandlerSV, %function
TIM8_CC_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl TIM8_CC_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size TIM8_CC_IRQHandlerSV, .-TIM8_CC_IRQHandlerSV

.section  .text.DMA1_Stream7_IRQHandlerSV
.weak DMA1_Stream7_IRQHandlerSV
.type DMA1_Stream7_IRQHandlerSV, %function
DMA1_Stream7_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl DMA1_Stream7_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size DMA1_Stream7_IRQHandlerSV, .-DMA1_Stream7_IRQHandlerSV

.section  .text.FSMC_IRQHandlerSV
.weak FSMC_IRQHandlerSV
.type FSMC_IRQHandlerSV, %function
FSMC_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl FSMC_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size FSMC_IRQHandlerSV, .-FSMC_IRQHandlerSV

.section  .text.SDIO_IRQHandlerSV
.weak SDIO_IRQHandlerSV
.type SDIO_IRQHandlerSV, %function
SDIO_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl SDIO_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size SDIO_IRQHandlerSV, .-SDIO_IRQHandlerSV

.section  .text.TIM5_IRQHandlerSV
.weak TIM5_IRQHandlerSV
.type TIM5_IRQHandlerSV, %function
TIM5_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl TIM5_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size TIM5_IRQHandlerSV, .-TIM5_IRQHandlerSV

.section  .text.SPI3_IRQHandlerSV
.weak SPI3_IRQHandlerSV
.type SPI3_IRQHandlerSV, %function
SPI3_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl SPI3_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size SPI3_IRQHandlerSV, .-SPI3_IRQHandlerSV

.section  .text.UART4_IRQHandlerSV
.weak UART4_IRQHandlerSV
.type UART4_IRQHandlerSV, %function
UART4_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl UART4_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size UART4_IRQHandlerSV, .-UART4_IRQHandlerSV

.section  .text.UART5_IRQHandlerSV
.weak UART5_IRQHandlerSV
.type UART5_IRQHandlerSV, %function
UART5_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl UART5_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size UART5_IRQHandlerSV, .-UART5_IRQHandlerSV

.section  .text.TIM6_DAC_IRQHandlerSV
.weak TIM6_DAC_IRQHandlerSV
.type TIM6_DAC_IRQHandlerSV, %function
TIM6_DAC_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl TIM6_DAC_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size TIM6_DAC_IRQHandlerSV, .-TIM6_DAC_IRQHandlerSV

.section  .text.TIM7_IRQHandlerSV
.weak TIM7_IRQHandlerSV
.type TIM7_IRQHandlerSV, %function
TIM7_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl TIM7_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size TIM7_IRQHandlerSV, .-TIM7_IRQHandlerSV

.section  .text.DMA2_Stream0_IRQHandlerSV
.weak DMA2_Stream0_IRQHandlerSV
.type DMA2_Stream0_IRQHandlerSV, %function
DMA2_Stream0_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl DMA2_Stream0_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size DMA2_Stream0_IRQHandlerSV, .-DMA2_Stream0_IRQHandlerSV

.section  .text.DMA2_Stream1_IRQHandlerSV
.weak DMA2_Stream1_IRQHandlerSV
.type DMA2_Stream1_IRQHandlerSV, %function
DMA2_Stream1_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl DMA2_Stream1_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size DMA2_Stream1_IRQHandlerSV, .-DMA2_Stream1_IRQHandlerSV

.section  .text.DMA2_Stream2_IRQHandlerSV
.weak DMA2_Stream2_IRQHandlerSV
.type DMA2_Stream2_IRQHandlerSV, %function
DMA2_Stream2_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl DMA2_Stream2_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size DMA2_Stream2_IRQHandlerSV, .-DMA2_Stream2_IRQHandlerSV

.section  .text.DMA2_Stream3_IRQHandlerSV
.weak DMA2_Stream3_IRQHandlerSV
.type DMA2_Stream3_IRQHandlerSV, %function
DMA2_Stream3_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl DMA2_Stream3_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size DMA2_Stream3_IRQHandlerSV, .-DMA2_Stream3_IRQHandlerSV

.section  .text.DMA2_Stream4_IRQHandlerSV
.weak DMA2_Stream4_IRQHandlerSV
.type DMA2_Stream4_IRQHandlerSV, %function
DMA2_Stream4_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl DMA2_Stream4_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size DMA2_Stream4_IRQHandlerSV, .-DMA2_Stream4_IRQHandlerSV

.section  .text.ETH_IRQHandlerSV
.weak ETH_IRQHandlerSV
.type ETH_IRQHandlerSV, %function
ETH_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl ETH_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size ETH_IRQHandlerSV, .-ETH_IRQHandlerSV

.section  .text.ETH_WKUP_IRQHandlerSV
.weak ETH_WKUP_IRQHandlerSV
.type ETH_WKUP_IRQHandlerSV, %function
ETH_WKUP_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl ETH_WKUP_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size ETH_WKUP_IRQHandlerSV, .-ETH_WKUP_IRQHandlerSV

.section  .text.CAN2_TX_IRQHandlerSV
.weak CAN2_TX_IRQHandlerSV
.type CAN2_TX_IRQHandlerSV, %function
CAN2_TX_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl CAN2_TX_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size CAN2_TX_IRQHandlerSV, .-CAN2_TX_IRQHandlerSV

.section  .text.CAN2_RX0_IRQHandlerSV
.weak CAN2_RX0_IRQHandlerSV
.type CAN2_RX0_IRQHandlerSV, %function
CAN2_RX0_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl CAN2_RX0_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size CAN2_RX0_IRQHandlerSV, .-CAN2_RX0_IRQHandlerSV

.section  .text.CAN2_RX1_IRQHandlerSV
.weak CAN2_RX1_IRQHandlerSV
.type CAN2_RX1_IRQHandlerSV, %function
CAN2_RX1_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl CAN2_RX1_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size CAN2_RX1_IRQHandlerSV, .-CAN2_RX1_IRQHandlerSV

.section  .text.CAN2_SCE_IRQHandlerSV
.weak CAN2_SCE_IRQHandlerSV
.type CAN2_SCE_IRQHandlerSV, %function
CAN2_SCE_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl CAN2_SCE_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size CAN2_SCE_IRQHandlerSV, .-CAN2_SCE_IRQHandlerSV

.section  .text.OTG_FS_IRQHandlerSV
.weak OTG_FS_IRQHandlerSV
.type OTG_FS_IRQHandlerSV, %function
OTG_FS_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl OTG_FS_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size OTG_FS_IRQHandlerSV, .-OTG_FS_IRQHandlerSV

.section  .text.DMA2_Stream5_IRQHandlerSV
.weak DMA2_Stream5_IRQHandlerSV
.type DMA2_Stream5_IRQHandlerSV, %function
DMA2_Stream5_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl DMA2_Stream5_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size DMA2_Stream5_IRQHandlerSV, .-DMA2_Stream5_IRQHandlerSV

.section  .text.DMA2_Stream6_IRQHandlerSV
.weak DMA2_Stream6_IRQHandlerSV
.type DMA2_Stream6_IRQHandlerSV, %function
DMA2_Stream6_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl DMA2_Stream6_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size DMA2_Stream6_IRQHandlerSV, .-DMA2_Stream6_IRQHandlerSV

.section  .text.DMA2_Stream7_IRQHandlerSV
.weak DMA2_Stream7_IRQHandlerSV
.type DMA2_Stream7_IRQHandlerSV, %function
DMA2_Stream7_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl DMA2_Stream7_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size DMA2_Stream7_IRQHandlerSV, .-DMA2_Stream7_IRQHandlerSV

.section  .text.USART6_IRQHandlerSV
.weak USART6_IRQHandlerSV
.type USART6_IRQHandlerSV, %function
USART6_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl USART6_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size USART6_IRQHandlerSV, .-USART6_IRQHandlerSV

.section  .text.I2C3_EV_IRQHandlerSV
.weak I2C3_EV_IRQHandlerSV
.type I2C3_EV_IRQHandlerSV, %function
I2C3_EV_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl I2C3_EV_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size I2C3_EV_IRQHandlerSV, .-I2C3_EV_IRQHandlerSV

.section  .text.I2C3_ER_IRQHandlerSV
.weak I2C3_ER_IRQHandlerSV
.type I2C3_ER_IRQHandlerSV, %function
I2C3_ER_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl I2C3_ER_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size I2C3_ER_IRQHandlerSV, .-I2C3_ER_IRQHandlerSV

.section  .text.OTG_HS_EP1_OUT_IRQHandlerSV
.weak OTG_HS_EP1_OUT_IRQHandlerSV
.type OTG_HS_EP1_OUT_IRQHandlerSV, %function
OTG_HS_EP1_OUT_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl OTG_HS_EP1_OUT_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size OTG_HS_EP1_OUT_IRQHandlerSV, .-OTG_HS_EP1_OUT_IRQHandlerSV

.section  .text.OTG_HS_EP1_IN_IRQHandlerSV
.weak OTG_HS_EP1_IN_IRQHandlerSV
.type OTG_HS_EP1_IN_IRQHandlerSV, %function
OTG_HS_EP1_IN_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl OTG_HS_EP1_IN_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size OTG_HS_EP1_IN_IRQHandlerSV, .-OTG_HS_EP1_IN_IRQHandlerSV

.section  .text.OTG_HS_WKUP_IRQHandlerSV
.weak OTG_HS_WKUP_IRQHandlerSV
.type OTG_HS_WKUP_IRQHandlerSV, %function
OTG_HS_WKUP_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl OTG_HS_WKUP_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size OTG_HS_WKUP_IRQHandlerSV, .-OTG_HS_WKUP_IRQHandlerSV

.section  .text.OTG_HS_IRQHandlerSV
.weak OTG_HS_IRQHandlerSV
.type OTG_HS_IRQHandlerSV, %function
OTG_HS_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl OTG_HS_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size OTG_HS_IRQHandlerSV, .-OTG_HS_IRQHandlerSV

.section  .text.DCMI_IRQHandlerSV
.weak DCMI_IRQHandlerSV
.type DCMI_IRQHandlerSV, %function
DCMI_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl DCMI_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size DCMI_IRQHandlerSV, .-DCMI_IRQHandlerSV

.section  .text.CRYP_IRQHandlerSV
.weak CRYP_IRQHandlerSV
.type CRYP_IRQHandlerSV, %function
CRYP_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl CRYP_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size CRYP_IRQHandlerSV, .-CRYP_IRQHandlerSV

.section  .text.HASH_RNG_IRQHandlerSV
.weak HASH_RNG_IRQHandlerSV
.type HASH_RNG_IRQHandlerSV, %function
HASH_RNG_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl HASH_RNG_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size HASH_RNG_IRQHandlerSV, .-HASH_RNG_IRQHandlerSV

.section  .text.FPU_IRQHandlerSV
.weak FPU_IRQHandlerSV
.type FPU_IRQHandlerSV, %function
FPU_IRQHandlerSV:
    push    {r7,lr}
    sub     sp, #8
    add     r7, sp, #0
    bl SEGGER_SYSVIEW_RecordEnterISR
    bl FPU_IRQHandler
    bl SEGGER_SYSVIEW_RecordExitISR
    adds    r7, #8
    mov     sp, r7
    pop     {r7, pc}
.size FPU_IRQHandlerSV, .-FPU_IRQHandlerSV

#endif

/******************************************************************************
*
* The minimal vector table for a Cortex M3. Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
* 
*******************************************************************************/
   .section  .isr_vector,"a",%progbits
  .type  g_pfnVectors, %object
  .size  g_pfnVectors, .-g_pfnVectors
    
    
g_pfnVectors:
  .word  _estack
  .word  Reset_Handler
  .word  NMI_Handler
  .word  HardFault_Handler
  .word  MemManage_Handler
  .word  BusFault_Handler
  .word  UsageFault_Handler
  .word  0
  .word  0
  .word  0
  .word  0
  .word  SVC_Handler
  .word  DebugMon_Handler
  .word  0
  .word  PendSV_Handler
  .word  SysTick_Handler

#ifndef SYSVIEW
  /* External Interrupts */
  .word     WWDG_IRQHandler                   /* Window WatchDog              */
  .word     PVD_IRQHandler                    /* PVD through EXTI Line detection */
  .word     TAMP_STAMP_IRQHandler             /* Tamper and TimeStamps through the EXTI line */
  .word     RTC_WKUP_IRQHandler               /* RTC Wakeup through the EXTI line */
  .word     FLASH_IRQHandler                  /* FLASH                        */
  .word     RCC_IRQHandler                    /* RCC                          */
  .word     EXTI0_IRQHandler                  /* EXTI Line0                   */
  .word     EXTI1_IRQHandler                  /* EXTI Line1                   */
  .word     EXTI2_IRQHandler                  /* EXTI Line2                   */
  .word     EXTI3_IRQHandler                  /* EXTI Line3                   */
  .word     EXTI4_IRQHandler                  /* EXTI Line4                   */
  .word     DMA1_Stream0_IRQHandler           /* DMA1 Stream 0                */
  .word     DMA1_Stream1_IRQHandler           /* DMA1 Stream 1                */
  .word     DMA1_Stream2_IRQHandler           /* DMA1 Stream 2                */
  .word     DMA1_Stream3_IRQHandler           /* DMA1 Stream 3                */
  .word     DMA1_Stream4_IRQHandler           /* DMA1 Stream 4                */
  .word     DMA1_Stream5_IRQHandler           /* DMA1 Stream 5                */
  .word     DMA1_Stream6_IRQHandler           /* DMA1 Stream 6                */
  .word     ADC_IRQHandler                    /* ADC1, ADC2 and ADC3s         */
  .word     CAN1_TX_IRQHandler                /* CAN1 TX                      */
  .word     CAN1_RX0_IRQHandler               /* CAN1 RX0                     */
  .word     CAN1_RX1_IRQHandler               /* CAN1 RX1                     */
  .word     CAN1_SCE_IRQHandler               /* CAN1 SCE                     */
  .word     EXTI9_5_IRQHandler                /* External Line[9:5]s          */
  .word     TIM1_BRK_TIM9_IRQHandler          /* TIM1 Break and TIM9          */
  .word     TIM1_UP_TIM10_IRQHandler          /* TIM1 Update and TIM10        */
  .word     TIM1_TRG_COM_TIM11_IRQHandler     /* TIM1 Trigger and Commutation and TIM11 */
  .word     TIM1_CC_IRQHandler                /* TIM1 Capture Compare         */
  .word     TIM2_IRQHandler                   /* TIM2                         */
  .word     TIM3_IRQHandler                   /* TIM3                         */
  .word     TIM4_IRQHandler                   /* TIM4                         */
  .word     I2C1_EV_IRQHandler                /* I2C1 Event                   */
  .word     I2C1_ER_IRQHandler                /* I2C1 Error                   */
  .word     I2C2_EV_IRQHandler                /* I2C2 Event                   */
  .word     I2C2_ER_IRQHandler                /* I2C2 Error                   */
  .word     SPI1_IRQHandler                   /* SPI1                         */
  .word     SPI2_IRQHandler                   /* SPI2                         */
  .word     USART1_IRQHandler                 /* USART1                       */
  .word     USART2_IRQHandler                 /* USART2                       */
  .word     USART3_IRQHandler                 /* USART3                       */
  .word     EXTI15_10_IRQHandler              /* External Line[15:10]s        */
  .word     RTC_Alarm_IRQHandler              /* RTC Alarm (A and B) through EXTI Line */
  .word     OTG_FS_WKUP_IRQHandler            /* USB OTG FS Wakeup through EXTI line */
  .word     TIM8_BRK_TIM12_IRQHandler         /* TIM8 Break and TIM12         */
  .word     TIM8_UP_TIM13_IRQHandler          /* TIM8 Update and TIM13        */
  .word     TIM8_TRG_COM_TIM14_IRQHandler     /* TIM8 Trigger and Commutation and TIM14 */
  .word     TIM8_CC_IRQHandler                /* TIM8 Capture Compare         */
  .word     DMA1_Stream7_IRQHandler           /* DMA1 Stream7                 */
  .word     FSMC_IRQHandler                   /* FSMC                         */
  .word     SDIO_IRQHandler                   /* SDIO                         */
  .word     TIM5_IRQHandler                   /* TIM5                         */
  .word     SPI3_IRQHandler                   /* SPI3                         */
  .word     UART4_IRQHandler                  /* UART4                        */
  .word     UART5_IRQHandler                  /* UART5                        */
  .word     TIM6_DAC_IRQHandler               /* TIM6 and DAC1&2 underrun errors */
  .word     TIM7_IRQHandler                   /* TIM7                         */
  .word     DMA2_Stream0_IRQHandler           /* DMA2 Stream 0                */
  .word     DMA2_Stream1_IRQHandler           /* DMA2 Stream 1                */
  .word     DMA2_Stream2_IRQHandler           /* DMA2 Stream 2                */
  .word     DMA2_Stream3_IRQHandler           /* DMA2 Stream 3                */
  .word     DMA2_Stream4_IRQHandler           /* DMA2 Stream 4                */
  .word     ETH_IRQHandler                    /* Ethernet                     */
  .word     ETH_WKUP_IRQHandler               /* Ethernet Wakeup through EXTI line */
  .word     CAN2_TX_IRQHandler                /* CAN2 TX                      */
  .word     CAN2_RX0_IRQHandler               /* CAN2 RX0                     */
  .word     CAN2_RX1_IRQHandler               /* CAN2 RX1                     */
  .word     CAN2_SCE_IRQHandler               /* CAN2 SCE                     */
  .word     OTG_FS_IRQHandler                 /* USB OTG FS                   */
  .word     DMA2_Stream5_IRQHandler           /* DMA2 Stream 5                */
  .word     DMA2_Stream6_IRQHandler           /* DMA2 Stream 6                */
  .word     DMA2_Stream7_IRQHandler           /* DMA2 Stream 7                */
  .word     USART6_IRQHandler                 /* USART6                       */
  .word     I2C3_EV_IRQHandler                /* I2C3 event                   */
  .word     I2C3_ER_IRQHandler                /* I2C3 error                   */
  .word     OTG_HS_EP1_OUT_IRQHandler         /* USB OTG HS End Point 1 Out   */
  .word     OTG_HS_EP1_IN_IRQHandler          /* USB OTG HS End Point 1 In    */
  .word     OTG_HS_WKUP_IRQHandler            /* USB OTG HS Wakeup through EXTI */
  .word     OTG_HS_IRQHandler                 /* USB OTG HS                   */
  .word     DCMI_IRQHandler                   /* DCMI                         */
  .word     CRYP_IRQHandler                   /* CRYP crypto                  */
  .word     HASH_RNG_IRQHandler               /* Hash and Rng                 */
  .word     FPU_IRQHandler                    /* FPU                          */

#else
// trace external interrupts with sysview
  /* External Interrupts */
  .word     WWDG_IRQHandlerSV                   /* Window WatchDog              */                                        
  .word     PVD_IRQHandlerSV                    /* PVD through EXTI Line detection */                        
  .word     TAMP_STAMP_IRQHandlerSV             /* Tamper and TimeStamps through the EXTI line */            
  .word     RTC_WKUP_IRQHandlerSV               /* RTC Wakeup through the EXTI line */                      
  .word     FLASH_IRQHandlerSV                  /* FLASH                        */                                          
  .word     RCC_IRQHandlerSV                    /* RCC                          */                                            
  .word     EXTI0_IRQHandlerSV                  /* EXTI Line0                   */
  .word     EXTI1_IRQHandlerSV                  /* EXTI Line1                   */                          
  .word     EXTI2_IRQHandlerSV                  /* EXTI Line2                   */                          
  .word     EXTI3_IRQHandlerSV                  /* EXTI Line3                   */                          
  .word     EXTI4_IRQHandlerSV                  /* EXTI Line4                   */                          
  .word     DMA1_Stream0_IRQHandlerSV           /* DMA1 Stream 0                */                  
  .word     DMA1_Stream1_IRQHandlerSV           /* DMA1 Stream 1                */                   
  .word     DMA1_Stream2_IRQHandlerSV           /* DMA1 Stream 2                */                   
  .word     DMA1_Stream3_IRQHandlerSV           /* DMA1 Stream 3                */                   
  .word     DMA1_Stream4_IRQHandlerSV           /* DMA1 Stream 4                */                   
  .word     DMA1_Stream5_IRQHandlerSV           /* DMA1 Stream 5                */                   
  .word     DMA1_Stream6_IRQHandlerSV           /* DMA1 Stream 6                */                   
  .word     ADC_IRQHandlerSV                    /* ADC1, ADC2 and ADC3s         */                   
  .word     CAN1_TX_IRQHandlerSV                /* CAN1 TX                      */                         
  .word     CAN1_RX0_IRQHandlerSV               /* CAN1 RX0                     */                          
  .word     CAN1_RX1_IRQHandlerSV               /* CAN1 RX1                     */                          
  .word     CAN1_SCE_IRQHandlerSV               /* CAN1 SCE                     */                          
  .word     EXTI9_5_IRQHandlerSV                /* External Line[9:5]s          */                          
  .word     TIM1_BRK_TIM9_IRQHandlerSV          /* TIM1 Break and TIM9          */         
  .word     TIM1_UP_TIM10_IRQHandlerSV          /* TIM1 Update and TIM10        */         
  .word     TIM1_TRG_COM_TIM11_IRQHandlerSV     /* TIM1 Trigger and Commutation and TIM11 */
  .word     TIM1_CC_IRQHandlerSV                /* TIM1 Capture Compare         */                          
  .word     TIM2_IRQHandlerSV                   /* TIM2                         */                   
  .word     TIM3_IRQHandlerSV                   /* TIM3                         */                   
  .word     TIM4_IRQHandlerSV                   /* TIM4                         */                   
  .word     I2C1_EV_IRQHandlerSV                /* I2C1 Event                   */                          
  .word     I2C1_ER_IRQHandlerSV                /* I2C1 Error                   */                          
  .word     I2C2_EV_IRQHandlerSV                /* I2C2 Event                   */                          
  .word     I2C2_ER_IRQHandlerSV                /* I2C2 Error                   */                            
  .word     SPI1_IRQHandlerSV                   /* SPI1                         */                   
  .word     SPI2_IRQHandlerSV                   /* SPI2                         */                   
  .word     USART1_IRQHandlerSV                 /* USART1                       */                   
  .word     USART2_IRQHandlerSV                 /* USART2                       */                   
  .word     USART3_IRQHandlerSV                 /* USART3                       */                   
  .word     EXTI15_10_IRQHandlerSV              /* External Line[15:10]s        */                          
  .word     RTC_Alarm_IRQHandlerSV              /* RTC Alarm (A and B) through EXTI Line */                 
  .word     OTG_FS_WKUP_IRQHandlerSV            /* USB OTG FS Wakeup through EXTI line */                       
  .word     TIM8_BRK_TIM12_IRQHandlerSV         /* TIM8 Break and TIM12         */         
  .word     TIM8_UP_TIM13_IRQHandlerSV          /* TIM8 Update and TIM13        */         
  .word     TIM8_TRG_COM_TIM14_IRQHandlerSV     /* TIM8 Trigger and Commutation and TIM14 */
  .word     TIM8_CC_IRQHandlerSV                /* TIM8 Capture Compare         */                          
  .word     DMA1_Stream7_IRQHandlerSV           /* DMA1 Stream7                 */                          
  .word     FSMC_IRQHandlerSV                   /* FSMC                         */                   
  .word     SDIO_IRQHandlerSV                   /* SDIO                         */                   
  .word     TIM5_IRQHandlerSV                   /* TIM5                         */                   
  .word     SPI3_IRQHandlerSV                   /* SPI3                         */                   
  .word     UART4_IRQHandlerSV                  /* UART4                        */                   
  .word     UART5_IRQHandlerSV                  /* UART5                        */                   
  .word     TIM6_DAC_IRQHandlerSV               /* TIM6 and DAC1&2 underrun errors */                   
  .word     TIM7_IRQHandlerSV                   /* TIM7                         */
  .word     DMA2_Stream0_IRQHandlerSV           /* DMA2 Stream 0                */                   
  .word     DMA2_Stream1_IRQHandlerSV           /* DMA2 Stream 1                */                   
  .word     DMA2_Stream2_IRQHandlerSV           /* DMA2 Stream 2                */                   
  .word     DMA2_Stream3_IRQHandlerSV           /* DMA2 Stream 3                */                   
  .word     DMA2_Stream4_IRQHandlerSV           /* DMA2 Stream 4                */                   
  .word     ETH_IRQHandlerSV                    /* Ethernet                     */                   
  .word     ETH_WKUP_IRQHandlerSV               /* Ethernet Wakeup through EXTI line */                     
  .word     CAN2_TX_IRQHandlerSV                /* CAN2 TX                      */                          
  .word     CAN2_RX0_IRQHandlerSV               /* CAN2 RX0                     */                          
  .word     CAN2_RX1_IRQHandlerSV               /* CAN2 RX1                     */                          
  .word     CAN2_SCE_IRQHandlerSV               /* CAN2 SCE                     */                          
  .word     OTG_FS_IRQHandlerSV                 /* USB OTG FS                   */                   
  .word     DMA2_Stream5_IRQHandlerSV           /* DMA2 Stream 5                */                   
  .word     DMA2_Stream6_IRQHandlerSV           /* DMA2 Stream 6                */                   
  .word     DMA2_Stream7_IRQHandlerSV           /* DMA2 Stream 7                */                   
  .word     USART6_IRQHandlerSV                 /* USART6                       */                    
  .word     I2C3_EV_IRQHandlerSV                /* I2C3 event                   */                          
  .word     I2C3_ER_IRQHandlerSV                /* I2C3 error                   */                          
  .word     OTG_HS_EP1_OUT_IRQHandlerSV         /* USB OTG HS End Point 1 Out   */                   
  .word     OTG_HS_EP1_IN_IRQHandlerSV          /* USB OTG HS End Point 1 In    */                   
  .word     OTG_HS_WKUP_IRQHandlerSV            /* USB OTG HS Wakeup through EXTI */                         
  .word     OTG_HS_IRQHandlerSV                 /* USB OTG HS                   */                   
  .word     DCMI_IRQHandlerSV                   /* DCMI                         */                   
  .word     CRYP_IRQHandlerSV                   /* CRYP crypto                  */                   
  .word     HASH_RNG_IRQHandlerSV               /* Hash and Rng                 */
  .word     FPU_IRQHandlerSV                    /* FPU                          */                         

#endif /* SYSVIEW */
                            
/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler. 
* As they are weak aliases, any function with the same name will override 
* this definition.
* 
*******************************************************************************/
   .weak      NMI_Handler
   .thumb_set NMI_Handler,Default_Handler
  
   .weak      HardFault_Handler
   .thumb_set HardFault_Handler,Default_Handler
  
   .weak      MemManage_Handler
   .thumb_set MemManage_Handler,Default_Handler
  
   .weak      BusFault_Handler
   .thumb_set BusFault_Handler,Default_Handler

   .weak      UsageFault_Handler
   .thumb_set UsageFault_Handler,Default_Handler

   .weak      SVC_Handler
   .thumb_set SVC_Handler,Default_Handler

   .weak      DebugMon_Handler
   .thumb_set DebugMon_Handler,Default_Handler

   .weak      PendSV_Handler
   .thumb_set PendSV_Handler,Default_Handler

   .weak      SysTick_Handler
   .thumb_set SysTick_Handler,Default_Handler              
  
   .weak      WWDG_IRQHandler                   
   .thumb_set WWDG_IRQHandler,Default_Handler      
                  
   .weak      PVD_IRQHandler      
   .thumb_set PVD_IRQHandler,Default_Handler
               
   .weak      TAMP_STAMP_IRQHandler            
   .thumb_set TAMP_STAMP_IRQHandler,Default_Handler
            
   .weak      RTC_WKUP_IRQHandler                  
   .thumb_set RTC_WKUP_IRQHandler,Default_Handler
            
   .weak      FLASH_IRQHandler         
   .thumb_set FLASH_IRQHandler,Default_Handler
                  
   .weak      RCC_IRQHandler      
   .thumb_set RCC_IRQHandler,Default_Handler
                  
   .weak      EXTI0_IRQHandler         
   .thumb_set EXTI0_IRQHandler,Default_Handler
                  
   .weak      EXTI1_IRQHandler         
   .thumb_set EXTI1_IRQHandler,Default_Handler
                     
   .weak      EXTI2_IRQHandler         
   .thumb_set EXTI2_IRQHandler,Default_Handler 
                 
   .weak      EXTI3_IRQHandler         
   .thumb_set EXTI3_IRQHandler,Default_Handler
                        
   .weak      EXTI4_IRQHandler         
   .thumb_set EXTI4_IRQHandler,Default_Handler
                  
   .weak      DMA1_Stream0_IRQHandler               
   .thumb_set DMA1_Stream0_IRQHandler,Default_Handler
         
   .weak      DMA1_Stream1_IRQHandler               
   .thumb_set DMA1_Stream1_IRQHandler,Default_Handler
                  
   .weak      DMA1_Stream2_IRQHandler               
   .thumb_set DMA1_Stream2_IRQHandler,Default_Handler
                  
   .weak      DMA1_Stream3_IRQHandler               
   .thumb_set DMA1_Stream3_IRQHandler,Default_Handler 
                 
   .weak      DMA1_Stream4_IRQHandler              
   .thumb_set DMA1_Stream4_IRQHandler,Default_Handler
                  
   .weak      DMA1_Stream5_IRQHandler               
   .thumb_set DMA1_Stream5_IRQHandler,Default_Handler
                  
   .weak      DMA1_Stream6_IRQHandler               
   .thumb_set DMA1_Stream6_IRQHandler,Default_Handler
                  
   .weak      ADC_IRQHandler      
   .thumb_set ADC_IRQHandler,Default_Handler
               
   .weak      CAN1_TX_IRQHandler   
   .thumb_set CAN1_TX_IRQHandler,Default_Handler
            
   .weak      CAN1_RX0_IRQHandler                  
   .thumb_set CAN1_RX0_IRQHandler,Default_Handler
                           
   .weak      CAN1_RX1_IRQHandler                  
   .thumb_set CAN1_RX1_IRQHandler,Default_Handler
            
   .weak      CAN1_SCE_IRQHandler                  
   .thumb_set CAN1_SCE_IRQHandler,Default_Handler
            
   .weak      EXTI9_5_IRQHandler   
   .thumb_set EXTI9_5_IRQHandler,Default_Handler
            
   .weak      TIM1_BRK_TIM9_IRQHandler            
   .thumb_set TIM1_BRK_TIM9_IRQHandler,Default_Handler
            
   .weak      TIM1_UP_TIM10_IRQHandler            
   .thumb_set TIM1_UP_TIM10_IRQHandler,Default_Handler
      
   .weak      TIM1_TRG_COM_TIM11_IRQHandler      
   .thumb_set TIM1_TRG_COM_TIM11_IRQHandler,Default_Handler
      
   .weak      TIM1_CC_IRQHandler   
   .thumb_set TIM1_CC_IRQHandler,Default_Handler
                  
   .weak      TIM2_IRQHandler            
   .thumb_set TIM2_IRQHandler,Default_Handler
                  
   .weak      TIM3_IRQHandler            
   .thumb_set TIM3_IRQHandler,Default_Handler
                  
   .weak      TIM4_IRQHandler            
   .thumb_set TIM4_IRQHandler,Default_Handler
                  
   .weak      I2C1_EV_IRQHandler   
   .thumb_set I2C1_EV_IRQHandler,Default_Handler
                     
   .weak      I2C1_ER_IRQHandler   
   .thumb_set I2C1_ER_IRQHandler,Default_Handler
                     
   .weak      I2C2_EV_IRQHandler   
   .thumb_set I2C2_EV_IRQHandler,Default_Handler
                  
   .weak      I2C2_ER_IRQHandler   
   .thumb_set I2C2_ER_IRQHandler,Default_Handler
                           
   .weak      SPI1_IRQHandler            
   .thumb_set SPI1_IRQHandler,Default_Handler
                        
   .weak      SPI2_IRQHandler            
   .thumb_set SPI2_IRQHandler,Default_Handler
                  
   .weak      USART1_IRQHandler      
   .thumb_set USART1_IRQHandler,Default_Handler
                     
   .weak      USART2_IRQHandler      
   .thumb_set USART2_IRQHandler,Default_Handler
                     
   .weak      USART3_IRQHandler      
   .thumb_set USART3_IRQHandler,Default_Handler
                  
   .weak      EXTI15_10_IRQHandler               
   .thumb_set EXTI15_10_IRQHandler,Default_Handler
               
   .weak      RTC_Alarm_IRQHandler               
   .thumb_set RTC_Alarm_IRQHandler,Default_Handler
            
   .weak      OTG_FS_WKUP_IRQHandler         
   .thumb_set OTG_FS_WKUP_IRQHandler,Default_Handler
            
   .weak      TIM8_BRK_TIM12_IRQHandler         
   .thumb_set TIM8_BRK_TIM12_IRQHandler,Default_Handler
         
   .weak      TIM8_UP_TIM13_IRQHandler            
   .thumb_set TIM8_UP_TIM13_IRQHandler,Default_Handler
         
   .weak      TIM8_TRG_COM_TIM14_IRQHandler      
   .thumb_set TIM8_TRG_COM_TIM14_IRQHandler,Default_Handler
      
   .weak      TIM8_CC_IRQHandler   
   .thumb_set TIM8_CC_IRQHandler,Default_Handler
                  
   .weak      DMA1_Stream7_IRQHandler               
   .thumb_set DMA1_Stream7_IRQHandler,Default_Handler
                     
   .weak      FSMC_IRQHandler            
   .thumb_set FSMC_IRQHandler,Default_Handler
                     
   .weak      SDIO_IRQHandler            
   .thumb_set SDIO_IRQHandler,Default_Handler
                     
   .weak      TIM5_IRQHandler            
   .thumb_set TIM5_IRQHandler,Default_Handler
                     
   .weak      SPI3_IRQHandler            
   .thumb_set SPI3_IRQHandler,Default_Handler
                     
   .weak      UART4_IRQHandler         
   .thumb_set UART4_IRQHandler,Default_Handler
                  
   .weak      UART5_IRQHandler         
   .thumb_set UART5_IRQHandler,Default_Handler
                  
   .weak      TIM6_DAC_IRQHandler                  
   .thumb_set TIM6_DAC_IRQHandler,Default_Handler
               
   .weak      TIM7_IRQHandler            
   .thumb_set TIM7_IRQHandler,Default_Handler
         
   .weak      DMA2_Stream0_IRQHandler               
   .thumb_set DMA2_Stream0_IRQHandler,Default_Handler
               
   .weak      DMA2_Stream1_IRQHandler               
   .thumb_set DMA2_Stream1_IRQHandler,Default_Handler
                  
   .weak      DMA2_Stream2_IRQHandler               
   .thumb_set DMA2_Stream2_IRQHandler,Default_Handler
            
   .weak      DMA2_Stream3_IRQHandler               
   .thumb_set DMA2_Stream3_IRQHandler,Default_Handler
            
   .weak      DMA2_Stream4_IRQHandler               
   .thumb_set DMA2_Stream4_IRQHandler,Default_Handler
            
   .weak      ETH_IRQHandler      
   .thumb_set ETH_IRQHandler,Default_Handler
                  
   .weak      ETH_WKUP_IRQHandler                  
   .thumb_set ETH_WKUP_IRQHandler,Default_Handler
            
   .weak      CAN2_TX_IRQHandler   
   .thumb_set CAN2_TX_IRQHandler,Default_Handler
                           
   .weak      CAN2_RX0_IRQHandler                  
   .thumb_set CAN2_RX0_IRQHandler,Default_Handler
                           
   .weak      CAN2_RX1_IRQHandler                  
   .thumb_set CAN2_RX1_IRQHandler,Default_Handler
                           
   .weak      CAN2_SCE_IRQHandler                  
   .thumb_set CAN2_SCE_IRQHandler,Default_Handler
                           
   .weak      OTG_FS_IRQHandler      
   .thumb_set OTG_FS_IRQHandler,Default_Handler
                     
   .weak      DMA2_Stream5_IRQHandler               
   .thumb_set DMA2_Stream5_IRQHandler,Default_Handler
                  
   .weak      DMA2_Stream6_IRQHandler               
   .thumb_set DMA2_Stream6_IRQHandler,Default_Handler
                  
   .weak      DMA2_Stream7_IRQHandler               
   .thumb_set DMA2_Stream7_IRQHandler,Default_Handler
                  
   .weak      USART6_IRQHandler      
   .thumb_set USART6_IRQHandler,Default_Handler
                        
   .weak      I2C3_EV_IRQHandler   
   .thumb_set I2C3_EV_IRQHandler,Default_Handler
                        
   .weak      I2C3_ER_IRQHandler   
   .thumb_set I2C3_ER_IRQHandler,Default_Handler
                        
   .weak      OTG_HS_EP1_OUT_IRQHandler         
   .thumb_set OTG_HS_EP1_OUT_IRQHandler,Default_Handler
               
   .weak      OTG_HS_EP1_IN_IRQHandler            
   .thumb_set OTG_HS_EP1_IN_IRQHandler,Default_Handler
               
   .weak      OTG_HS_WKUP_IRQHandler         
   .thumb_set OTG_HS_WKUP_IRQHandler,Default_Handler
            
   .weak      OTG_HS_IRQHandler      
   .thumb_set OTG_HS_IRQHandler,Default_Handler
                  
   .weak      DCMI_IRQHandler            
   .thumb_set DCMI_IRQHandler,Default_Handler
                     
   .weak      CRYP_IRQHandler            
   .thumb_set CRYP_IRQHandler,Default_Handler
               
   .weak      HASH_RNG_IRQHandler                  
   .thumb_set HASH_RNG_IRQHandler,Default_Handler   

   .weak      FPU_IRQHandler                  
   .thumb_set FPU_IRQHandler,Default_Handler 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
