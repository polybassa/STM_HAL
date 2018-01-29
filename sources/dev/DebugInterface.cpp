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

#include "DebugInterface.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <string>
#if defined (STM32F303xC) || defined (STM32F334x8) || defined (STM32F302x8) || defined (STM32F303xE)
#include "stm32f30x_rcc.h"
#endif
#if defined (STM32F10X_LD) || defined (STM32F10X_LD_VL) || defined (STM32F10X_MD) || defined (STM32F10X_MD_VL) || \
    defined (STM32F10X_HD) || defined (STM32F10X_HD_VL) || defined (STM32F10X_XL) || defined (STM32F10X_CL)
#include "stm32f10x_rcc.h"
#endif

#include "hal_Factory.h"

using dev::DebugInterface;

#if defined(DEBUG)
os::Mutex DebugInterface::PrintMutex;
std::array<uint8_t, DebugInterface::INTERNAL_BUFFER_SIZE> DebugInterface::printBuffer;
#endif

DebugInterface::DebugInterface(void)
{
    if (interface.isInitalized()) {
        clearTerminal();
        printStartupMessage();
    }
}

DebugInterface::~DebugInterface()
{
    const uint8_t closeMsg[] = "Debug Interface shutdown!";
    interface.send(closeMsg, sizeof(closeMsg));
}

void DebugInterface::printStartupMessage(void) const
{
    this->printf("Firmware\r\nBuild: %s %s\r\n", __DATE__, __TIME__);
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);

#if defined (STM32F303xC) || defined (STM32F334x8) || defined (STM32F302x8) || defined (STM32F303xE)
    this->printf("ADC12CLK: %d\r\n", clocks.ADC12CLK_Frequency);
    this->printf("ADC34CLK: %d\r\n", clocks.ADC34CLK_Frequency);
    this->printf("HCLK: %d\r\n", clocks.HCLK_Frequency);
    this->printf("HRTIM1CLK: %d\r\n", clocks.HRTIM1CLK_Frequency);
    this->printf("I2C1CLK: %d\r\n", clocks.I2C1CLK_Frequency);
    this->printf("I2C2CLK: %d\r\n", clocks.I2C2CLK_Frequency);
    this->printf("I2C3CLK: %d\r\n", clocks.I2C3CLK_Frequency);
    this->printf("PCLK1: %d\r\n", clocks.PCLK1_Frequency);
    this->printf("PCLK2: %d\r\n", clocks.PCLK2_Frequency);
    this->printf("SYSCLK: %d\r\n", clocks.SYSCLK_Frequency);
    this->printf("TIM15CLK: %d\r\n", clocks.TIM15CLK_Frequency);
    this->printf("TIM16CLK: %d\r\n", clocks.TIM16CLK_Frequency);
    this->printf("TIM17CLK: %d\r\n", clocks.TIM17CLK_Frequency);
    this->printf("TIM1CLK: %d\r\n", clocks.TIM1CLK_Frequency);
    this->printf("TIM20CLK: %d\r\n", clocks.TIM20CLK_Frequency);
    this->printf("TIM2CLK: %d\r\n", clocks.TIM2CLK_Frequency);
    this->printf("TIM3CLK: %d\r\n", clocks.TIM3CLK_Frequency);
    this->printf("TIM8CLK: %d\r\n", clocks.TIM8CLK_Frequency);
    this->printf("UART4CLK: %d\r\n", clocks.UART4CLK_Frequency);
    this->printf("UART5CLK: %d\r\n", clocks.UART5CLK_Frequency);
    this->printf("USART1CLK: %d\r\n", clocks.USART1CLK_Frequency);
    this->printf("USART2CLK: %d\r\n", clocks.USART2CLK_Frequency);
    this->printf("USART3CLK: %d\r\n", clocks.USART3CLK_Frequency);
#endif
#if defined (STM32F10X_LD) || defined (STM32F10X_LD_VL) || defined (STM32F10X_MD) || defined (STM32F10X_MD_VL) || \
    defined (STM32F10X_HD) || defined (STM32F10X_HD_VL) || defined (STM32F10X_XL) || defined (STM32F10X_CL)
    this->printf("HCLK: %d\r\n", clocks.HCLK_Frequency);
    this->printf("PCLK1: %d\r\n", clocks.PCLK1_Frequency);
    this->printf("PCLK2: %d\r\n", clocks.PCLK2_Frequency);
    this->printf("SYSCLK: %d\r\n", clocks.SYSCLK_Frequency);

#endif
}

void DebugInterface::clearTerminal(void) const
{
    const uint8_t clearMsg[] = "\33[2J\r";
    interface.send(clearMsg, sizeof(clearMsg));
}

bool DebugInterface::dataAvailable(void) const
{
    return interface.isReadyToReceive();
}

size_t DebugInterface::receiveAvailableData(uint8_t* const data, const size_t length) const
{
    return interface.receiveAvailableData(data, length);
}

void DebugInterface::send(const char* str, const size_t len) const
{
    this->interface.send(reinterpret_cast<const uint8_t*>(str), len);
}

void DebugInterface::printf(const char* format, ...) const
{
#if defined(DEBUG)
    PrintMutex.take();
    va_list list;
    va_start(list, format);
    const int retval = vsnprintf(reinterpret_cast<char*>(printBuffer.data()), printBuffer.size(), format, list);
    va_end(list);

    if ((retval < 0) || ((size_t)retval >= printBuffer.size())) {
        const uint8_t errorMsg[] = "Message to long\r\n";
        this->interface.send(errorMsg, sizeof(errorMsg));
    } else {
        this->interface.send(printBuffer.data(), retval);
    }
    PrintMutex.give();
#endif
}
