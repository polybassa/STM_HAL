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

#include "Spi.h"
#include "trace.h"
#include "LockGuard.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR |
                                                        ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Factory;
using hal::Spi;

void Spi::initialize() const
{
    SPI_I2S_DeInit(reinterpret_cast<SPI_TypeDef*>(mPeripherie));

    SPI_Init(reinterpret_cast<SPI_TypeDef*>(mPeripherie), &mConfiguration);

    if (mConfiguration.SPI_NSS == SPI_NSS_Hard) {
        SPI_NSSPulseModeCmd(reinterpret_cast<SPI_TypeDef*>(mPeripherie),
                            ENABLE);
        SPI_SSOutputCmd(reinterpret_cast<SPI_TypeDef*>(mPeripherie), ENABLE);
        SPI_RxFIFOThresholdConfig(reinterpret_cast<SPI_TypeDef*>(mPeripherie), SPI_RxFIFOThreshold_QF);
    }
    SPI_Cmd(reinterpret_cast<SPI_TypeDef*>(mPeripherie), ENABLE);
}

size_t Spi::send(uint8_t const* const data, const size_t length) const
{
    if (data == nullptr) {
        return 0;
    }
    os::LockGuard<os::Mutex> lock(InterfaceAvailableMutex[static_cast<size_t>(mDescription)]);

    size_t bytesSend = 0;
    while ((size_t)bytesSend < length) {
        if (this->isReadyToSend()) {
            this->send(data[bytesSend]);
            bytesSend++;
        }
    }
    return bytesSend;
}

void Spi::send(const uint8_t data) const
{
    SPI_SendData8(reinterpret_cast<SPI_TypeDef*>(mPeripherie), data);
}

bool Spi::isReadyToReceive(void) const
{
    return (bool)SPI_I2S_GetFlagStatus(
               reinterpret_cast<SPI_TypeDef*>(mPeripherie), SPI_I2S_FLAG_RXNE);
}

size_t Spi::receive(uint8_t* const data, const size_t length) const
{
    if (data == nullptr) {
        return 0;
    }
    os::LockGuard<os::Mutex> lock(InterfaceAvailableMutex[static_cast<size_t>(mDescription)]);

    constexpr const uint8_t defaultValueForSend = 0xff;
    size_t bytesSend = 0;
    while ((size_t)bytesSend < length) {
        if (this->isReadyToSend()) {
            this->send(defaultValueForSend);
        }
        if (this->isReadyToReceive()) {
            data[bytesSend] = this->receive();
            bytesSend++;
        }
    }
    return bytesSend;
}

uint8_t Spi::receive(void) const
{
    return SPI_ReceiveData8(reinterpret_cast<SPI_TypeDef*>(mPeripherie));
}

bool Spi::isReadyToSend(void) const
{
    return (bool)SPI_I2S_GetFlagStatus(
               reinterpret_cast<SPI_TypeDef*>(mPeripherie), SPI_I2S_FLAG_TXE);
}

std::array<os::Mutex, Spi::Description::__ENUM__SIZE> Spi::InterfaceAvailableMutex;
constexpr const std::array<const Spi, Spi::__ENUM__SIZE> Factory<Spi>::Container;
constexpr const std::array<const uint32_t, Spi::__ENUM__SIZE> Factory<Spi>::Clocks;
