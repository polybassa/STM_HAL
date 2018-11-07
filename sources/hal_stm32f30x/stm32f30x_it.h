// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef __STM32F10X_IT_H
#define __STM32F10X_IT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f30x.h"

void NMI_Handler(void);
void HardFault_Handler(void) __attribute__((naked));
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void DebugMon_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __STM32F10X_IT_H */
