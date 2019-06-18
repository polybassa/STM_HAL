// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "SEGGER_RTT.h"
#if defined (STM32F303xC) || defined (STM32F334x8) || defined (STM32F302x8) || defined (STM32F303xE)
#include "stm32f30x_rcc.h"
#endif
#if defined (STM32F10X_LD) || defined (STM32F10X_LD_VL) || defined (STM32F10X_MD) || defined (STM32F10X_MD_VL) || \
    defined (STM32F10X_HD) || defined (STM32F10X_HD_VL) || defined (STM32F10X_XL) || defined (STM32F10X_CL)
#include "stm32f10x_rcc.h"
#endif
#if defined(STM32F40_41xxx) || defined(STM32F427_437xx) || defined(STM32F429_439xx) || defined(STM32F401xx) || \
    defined(STM32F410xx) || \
    defined(STM32F411xE) || defined(STM32F412xG) || defined(STM32F413_423xx) || defined(STM32F446xx) || \
    defined(STM32F469_479xx)
#include "stm32f4xx_rcc.h"
#endif
#include <utility>

namespace dev
{
class RealTimeDebugInterface
{
    RealTimeDebugInterface(void)
    {
        SEGGER_RTT_ConfigUpBuffer(0,
                                  nullptr,
                                  nullptr,
                                  0,
                                  SEGGER_RTT_MODE_NO_BLOCK_TRIM);
    }
public:
    RealTimeDebugInterface(const RealTimeDebugInterface&) = delete;
    RealTimeDebugInterface(RealTimeDebugInterface&&) = delete;
    RealTimeDebugInterface& operator=(const RealTimeDebugInterface&) = delete;
    RealTimeDebugInterface& operator=(RealTimeDebugInterface&&) = delete;

    template<typename ... Params>
    void printf(Params&& ... params)
    {
        SEGGER_RTT_printf(0, std::forward<Params>(params) ...);
    }

    static RealTimeDebugInterface& instance()
    {
        static RealTimeDebugInterface _instance;
        return _instance;
    }

    void printStartupMessage(void)
    {
        SEGGER_RTT_printf(0, "Firmware\r\nBuild: %s %s\r\n", __DATE__, __TIME__);
        RCC_ClocksTypeDef clocks;
        RCC_GetClocksFreq(&clocks);

#if defined (STM32F303xC) || defined (STM32F334x8) || defined (STM32F302x8) || defined (STM32F303xE)
        SEGGER_RTT_printf(0, "ADC12CLK: %d\r\n", clocks.ADC12CLK_Frequency);
        SEGGER_RTT_printf(0, "ADC34CLK: %d\r\n", clocks.ADC34CLK_Frequency);
        SEGGER_RTT_printf(0, "HCLK: %d\r\n", clocks.HCLK_Frequency);
        SEGGER_RTT_printf(0, "HRTIM1CLK: %d\r\n", clocks.HRTIM1CLK_Frequency);
        SEGGER_RTT_printf(0, "I2C1CLK: %d\r\n", clocks.I2C1CLK_Frequency);
        SEGGER_RTT_printf(0, "I2C2CLK: %d\r\n", clocks.I2C2CLK_Frequency);
        SEGGER_RTT_printf(0, "I2C3CLK: %d\r\n", clocks.I2C3CLK_Frequency);
        SEGGER_RTT_printf(0, "PCLK1: %d\r\n", clocks.PCLK1_Frequency);
        SEGGER_RTT_printf(0, "PCLK2: %d\r\n", clocks.PCLK2_Frequency);
        SEGGER_RTT_printf(0, "SYSCLK: %d\r\n", clocks.SYSCLK_Frequency);
        SEGGER_RTT_printf(0, "TIM15CLK: %d\r\n", clocks.TIM15CLK_Frequency);
        SEGGER_RTT_printf(0, "TIM16CLK: %d\r\n", clocks.TIM16CLK_Frequency);
        SEGGER_RTT_printf(0, "TIM17CLK: %d\r\n", clocks.TIM17CLK_Frequency);
        SEGGER_RTT_printf(0, "TIM1CLK: %d\r\n", clocks.TIM1CLK_Frequency);
        SEGGER_RTT_printf(0, "TIM20CLK: %d\r\n", clocks.TIM20CLK_Frequency);
        SEGGER_RTT_printf(0, "TIM2CLK: %d\r\n", clocks.TIM2CLK_Frequency);
        SEGGER_RTT_printf(0, "TIM3CLK: %d\r\n", clocks.TIM3CLK_Frequency);
        SEGGER_RTT_printf(0, "TIM8CLK: %d\r\n", clocks.TIM8CLK_Frequency);
        SEGGER_RTT_printf(0, "UART4CLK: %d\r\n", clocks.UART4CLK_Frequency);
        SEGGER_RTT_printf(0, "UART5CLK: %d\r\n", clocks.UART5CLK_Frequency);
        SEGGER_RTT_printf(0, "USART1CLK: %d\r\n", clocks.USART1CLK_Frequency);
        SEGGER_RTT_printf(0, "USART2CLK: %d\r\n", clocks.USART2CLK_Frequency);
        SEGGER_RTT_printf(0, "USART3CLK: %d\r\n", clocks.USART3CLK_Frequency);
#endif
#if defined (STM32F10X_LD) || defined (STM32F10X_LD_VL) || defined (STM32F10X_MD) || defined (STM32F10X_MD_VL) || \
        defined (STM32F10X_HD) || defined (STM32F10X_HD_VL) || defined (STM32F10X_XL) || defined (STM32F10X_CL)
        SEGGER_RTT_printf(0, "HCLK: %d\r\n", clocks.HCLK_Frequency);
        SEGGER_RTT_printf(0, "PCLK1: %d\r\n", clocks.PCLK1_Frequency);
        SEGGER_RTT_printf(0, "PCLK2: %d\r\n", clocks.PCLK2_Frequency);
        SEGGER_RTT_printf(0, "SYSCLK: %d\r\n", clocks.SYSCLK_Frequency);

#endif
#if defined(STM32F40_41xxx) || defined(STM32F427_437xx) || defined(STM32F429_439xx) || defined(STM32F401xx) || \
        defined(STM32F410xx) || \
        defined(STM32F411xE) || defined(STM32F412xG) || defined(STM32F413_423xx) || defined(STM32F446xx) || \
        defined(STM32F469_479xx)
        SEGGER_RTT_printf(0, "HCLK: %d\r\n", clocks.HCLK_Frequency);
        SEGGER_RTT_printf(0, "PCLK1: %d\r\n", clocks.PCLK1_Frequency);
        SEGGER_RTT_printf(0, "PCLK2: %d\r\n", clocks.PCLK2_Frequency);
        SEGGER_RTT_printf(0, "SYSCLK: %d\r\n", clocks.SYSCLK_Frequency);
#endif
    }
};
}
