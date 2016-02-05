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
#include "stm32f30x_rcc.h"
#include <string>

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
    print("Firmware\r\nBuild: %s %s\r\n", __DATE__, __TIME__);
    RCC_ClocksTypeDef clocks;
    RCC_GetClocksFreq(&clocks);

    print("ADC12CLK: %d\r\n", clocks.ADC12CLK_Frequency);
    print("ADC34CLK: %d\r\n", clocks.ADC34CLK_Frequency);
    print("HCLK: %d\r\n", clocks.HCLK_Frequency);
    print("HRTIM1CLK: %d\r\n", clocks.HRTIM1CLK_Frequency);
    print("I2C1CLK: %d\r\n", clocks.I2C1CLK_Frequency);
    print("I2C2CLK: %d\r\n", clocks.I2C2CLK_Frequency);
    print("I2C3CLK: %d\r\n", clocks.I2C3CLK_Frequency);
    print("PCLK1: %d\r\n", clocks.PCLK1_Frequency);
    print("PCLK2: %d\r\n", clocks.PCLK2_Frequency);
    print("SYSCLK: %d\r\n", clocks.SYSCLK_Frequency);
    print("TIM15CLK: %d\r\n", clocks.TIM15CLK_Frequency);
    print("TIM16CLK: %d\r\n", clocks.TIM16CLK_Frequency);
    print("TIM17CLK: %d\r\n", clocks.TIM17CLK_Frequency);
    print("TIM1CLK: %d\r\n", clocks.TIM1CLK_Frequency);
    print("TIM20CLK: %d\r\n", clocks.TIM20CLK_Frequency);
    print("TIM2CLK: %d\r\n", clocks.TIM2CLK_Frequency);
    print("TIM3CLK: %d\r\n", clocks.TIM3CLK_Frequency);
    print("TIM8CLK: %d\r\n", clocks.TIM8CLK_Frequency);
    print("UART4CLK: %d\r\n", clocks.UART4CLK_Frequency);
    print("UART5CLK: %d\r\n", clocks.UART5CLK_Frequency);
    print("USART1CLK: %d\r\n", clocks.USART1CLK_Frequency);
    print("USART2CLK: %d\r\n", clocks.USART2CLK_Frequency);
    print("USART3CLK: %d\r\n", clocks.USART3CLK_Frequency);
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

void DebugInterface::print(const char* format, ...) const
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
