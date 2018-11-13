// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "stm32f30x_it.h"

void prvGetRegistersFromStack(uint32_t* pulFaultStackAddress)
{
/* These are volatile to try and prevent the compiler/linker optimising them
   away as the variables never actually get used.  If the debugger won't show the
   values of the variables, make them global my moving their declaration outside
   of this function. */
    volatile uint32_t __attribute__((unused)) r0;
    volatile uint32_t __attribute__((unused)) r1;
    volatile uint32_t __attribute__((unused)) r2;
    volatile uint32_t __attribute__((unused)) r3;
    volatile uint32_t __attribute__((unused)) r12;
    volatile uint32_t __attribute__((unused)) lr; /* Link register. */
    volatile uint32_t __attribute__((unused)) pc; /* Program counter. */
    volatile uint32_t __attribute__((unused)) psr; /* Program status register. */

    r0 = pulFaultStackAddress[ 0 ];
    r1 = pulFaultStackAddress[ 1 ];
    r2 = pulFaultStackAddress[ 2 ];
    r3 = pulFaultStackAddress[ 3 ];

    r12 = pulFaultStackAddress[ 4 ];
    lr = pulFaultStackAddress[ 5 ];
    pc = pulFaultStackAddress[ 6 ];
    psr = pulFaultStackAddress[ 7 ];

    /* When the following line is hit, the variables contain the register values. */
    for ( ; ; ) {}
}

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
 * @brief  This function handles NMI exception.
 * @param  None
 * @retval None
 */
void NMI_Handler(void)
{}

/**
 * @brief  This function handles Hard Fault exception.
 * @param  None
 * @retval None
 */
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */

    __asm volatile
    (
     " tst lr, #4                                                \n"
     " ite eq                                                    \n"
     " mrseq r0, msp                                             \n"
     " mrsne r0, psp                                             \n"
     " ldr r1, [r0, #24]                                         \n"
     " ldr r2, handler2_address_const                            \n"
     " bx r2                                                     \n"
     " handler2_address_const: .word prvGetRegistersFromStack    \n"
    );

    while (1) {}
}

/**
 * @brief  This function handles Memory Manage exception.
 * @param  None
 * @retval None
 */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1) {}
}

/**
 * @brief  This function handles Bus Fault exception.
 * @param  None
 * @retval None
 */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1) {}
}

/**
 * @brief  This function handles Usage Fault exception.
 * @param  None
 * @retval None
 */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1) {}
}

/**
 * @brief  This function handles Debug Monitor exception.
 * @param  None
 * @retval None
 */
void DebugMon_Handler(void)
{}

/**
 * @brief  This function handles PendSVC exception.
 * @param  None
 * @retval None
 */

/******************************************************************************/
/*                 STM32F30x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f30x.s).                                            */
/******************************************************************************/
/**
 * @brief  This function handles PPP interrupt request.
 * @param  None
 * @retval None
 */
/*void PPP_IRQHandler(void)
   {
   }*/
