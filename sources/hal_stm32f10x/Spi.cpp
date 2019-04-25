// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "Spi.h"
#include "trace.h"
#include "LockGuard.h"
#include "os_Task.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR |
                                                        ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Factory;
using hal::Spi;

void Spi::initialize() const
{
    SPI_I2S_DeInit(reinterpret_cast<SPI_TypeDef*>(mPeripherie));

    SPI_Init(reinterpret_cast<SPI_TypeDef*>(mPeripherie), &mConfiguration);

    if (mConfiguration.SPI_NSS == SPI_NSS_Hard) {
        SPI_SSOutputCmd(reinterpret_cast<SPI_TypeDef*>(mPeripherie), ENABLE);
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
    SPI_I2S_SendData(reinterpret_cast<SPI_TypeDef*>(mPeripherie), data);
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
    //clear RXNE flag
    this->receive();
    while ((size_t)bytesSend < length) {
        if (this->isReadyToSend()) {
            this->send(defaultValueForSend);
            while (this->isReadyToReceive() == false) {
                ; // will stuck here if peripherial goes down
            }
            data[bytesSend] = this->receive();
            bytesSend++;
        }
    }
    return bytesSend;
}

uint8_t Spi::receive(void) const
{
    return static_cast<uint8_t>(
                                SPI_I2S_ReceiveData(reinterpret_cast<SPI_TypeDef*>(mPeripherie)));
}

bool Spi::isReadyToSend(void) const
{
    return (bool)SPI_I2S_GetFlagStatus(reinterpret_cast<SPI_TypeDef*>(mPeripherie),
                                       SPI_I2S_FLAG_TXE);
}

bool Spi::isBusy(void) const
{
    return (bool)SPI_I2S_GetFlagStatus(reinterpret_cast<SPI_TypeDef*>(mPeripherie),
                                       SPI_I2S_FLAG_BSY);
}

std::array<os::Mutex, Spi::Description::__ENUM__SIZE> Spi::InterfaceAvailableMutex;
constexpr const std::array<const Spi, Spi::__ENUM__SIZE> Factory<Spi>::Container;
constexpr const std::array<const uint32_t, Spi::__ENUM__SIZE> Factory<Spi>::Clocks;
