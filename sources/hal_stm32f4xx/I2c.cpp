/* Copyright (C) 2015  Nils Weiss, Florian Breintner, Markus Wildgruber
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

#include "I2c.h"
#include "trace.h"
#include "LockGuard.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::I2c;

void I2c::initialize() const
{
    I2C_TypeDef* pPeripherie = reinterpret_cast<I2C_TypeDef*>(mPeripherie);
    if (!IS_I2C_ALL_PERIPH(pPeripherie)) {
        return;
    }

    I2C_DeInit(pPeripherie);

    I2C_InitTypeDef tmpConfiguration = mConfiguration;
    I2C_Init(pPeripherie, &tmpConfiguration);
    I2C_Cmd(pPeripherie, ENABLE);
}

bool I2c::timeoutDuringWaitUntilFlagIsEqualState(const uint32_t flag, const FlagStatus state) const
{
    /* Timeout for I2c write/read routines */
    static const uint32_t TIMEOUT_CYCLES = 0x10000;
    uint32_t timeoutCounter = TIMEOUT_CYCLES;
    while (I2C_GetFlagStatus(reinterpret_cast<I2C_TypeDef*>(mPeripherie), flag) != state) {
        if ((timeoutCounter--) == 0) {
            return true;
        }
    }
    return false;
}

bool I2c::timeoutDuringWaitEvent(const uint32_t event) const
{
    /* Timeout for I2c write/read routines */
    static const uint32_t TIMEOUT_CYCLES = 0x5555;
    uint32_t timeoutCounter = TIMEOUT_CYCLES;
    while (I2C_CheckEvent(reinterpret_cast<I2C_TypeDef*>(mPeripherie), event) != SUCCESS) {
        if ((timeoutCounter--) == 0) {
            return true;
        }
    }
    return false;
}

size_t I2c::write(const uint16_t deviceAddr, const uint8_t regAddr, uint8_t const* const data,
                  const size_t length) const
{
    // prepare communication
    os::LockGuard<os::Mutex> lock(MutexArray[static_cast<size_t>(mDescription)]);

    if (!sendRegisterAddress(deviceAddr, regAddr, length)) {
        return 0;
    }

    size_t bytesSend = 0;
    while (bytesSend < length) {
        if (timeoutDuringWaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
            return bytesSend;
        }
        I2C_SendData(reinterpret_cast<I2C_TypeDef*>(mPeripherie), data[bytesSend]);
        bytesSend++;
    }

    timeoutDuringWaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED);

    I2C_GenerateSTOP(reinterpret_cast<I2C_TypeDef*>(mPeripherie), ENABLE);

    return bytesSend;
}

size_t I2c::write(const uint16_t deviceAddr, uint8_t const* const data, const size_t length) const
{
    // prepare communication
    os::LockGuard<os::Mutex> lock(MutexArray[static_cast<size_t>(mDescription)]);
    if (!initMasterCommunication(deviceAddr, length)) {
        return false;
    }

    I2C_Send7bitAddress(reinterpret_cast<I2C_TypeDef*>(mPeripherie),
                        static_cast<uint8_t>(deviceAddr << 1),
                        I2C_Direction_Transmitter);

    if (timeoutDuringWaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
        I2C_GenerateSTOP(reinterpret_cast<I2C_TypeDef*>(mPeripherie), ENABLE);
        return false;
    }

    size_t bytesSend = 0;
    while (bytesSend < length) {
        I2C_SendData(reinterpret_cast<I2C_TypeDef*>(mPeripherie), data[bytesSend]);
        bytesSend++;
        if (timeoutDuringWaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) {
            return bytesSend;
        }
    }

    I2C_GenerateSTOP(reinterpret_cast<I2C_TypeDef*>(mPeripherie), ENABLE);

    return bytesSend;
}

size_t I2c::read(const uint16_t deviceAddr, const uint8_t regAddr, uint8_t* const data, const size_t length) const
{
    // prepare communication
    os::LockGuard<os::Mutex> lock(MutexArray[static_cast<size_t>(mDescription)]);

    if (!sendRegisterAddress(deviceAddr, regAddr, length)) {
        return 0;
    }

    // restart communication for reading
    I2C_GenerateSTART(reinterpret_cast<I2C_TypeDef*>(mPeripherie), ENABLE);

    if (timeoutDuringWaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) {
        I2C_GenerateSTOP(reinterpret_cast<I2C_TypeDef*>(mPeripherie), ENABLE);
        return false;
    }

    I2C_Send7bitAddress(reinterpret_cast<I2C_TypeDef*>(mPeripherie),
                        static_cast<uint8_t>(deviceAddr << 1),
                        I2C_Direction_Receiver);

    if (timeoutDuringWaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)) {
        I2C_GenerateSTOP(reinterpret_cast<I2C_TypeDef*>(mPeripherie), ENABLE);
        return 0;
    }

    size_t bytesReceived = 0;
    while (bytesReceived < length) {
        if (timeoutDuringWaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED)) {
            I2C_GenerateSTOP(reinterpret_cast<I2C_TypeDef*>(mPeripherie), ENABLE);
            return bytesReceived;
        }
        data[bytesReceived] = I2C_ReceiveData(reinterpret_cast<I2C_TypeDef*>(mPeripherie));
        bytesReceived++;
    }

    // end communication
    I2C_GenerateSTOP(reinterpret_cast<I2C_TypeDef*>(mPeripherie), ENABLE);

    return bytesReceived;
}

bool I2c::initMasterCommunication(const uint16_t deviceAddr, const size_t length) const
{
    if (length > 255) {
        Trace(ZONE_ERROR, "Transfer length greater than 255 not implemented, yet!");
        return false;
    }

    if ((deviceAddr & 0xFF80) != 0) {
        // 10 bit addressing
        Trace(ZONE_ERROR, "10 Bit addresses are not yet supported!");
        return false;
    }

    if (timeoutDuringWaitUntilFlagIsEqualState(I2C_FLAG_BUSY, RESET)) {
        return false;
    }

    I2C_GenerateSTART(reinterpret_cast<I2C_TypeDef*>(mPeripherie), ENABLE);

    if (timeoutDuringWaitEvent(I2C_EVENT_MASTER_MODE_SELECT)) {
        // end communication
        I2C_GenerateSTOP(reinterpret_cast<I2C_TypeDef*>(mPeripherie), ENABLE);
        return false;
    }

    return true;
}

bool I2c::sendRegisterAddress(const uint16_t deviceAddr, const uint8_t regAddr, const size_t length) const
{
    if (!initMasterCommunication(deviceAddr, length)) {
        return false;
    }

    I2C_Send7bitAddress(reinterpret_cast<I2C_TypeDef*>(mPeripherie),
                        static_cast<uint8_t>(deviceAddr << 1),
                        I2C_Direction_Transmitter);

    if (timeoutDuringWaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) {
        I2C_GenerateSTOP(reinterpret_cast<I2C_TypeDef*>(mPeripherie), ENABLE);
        return false;
    }

    I2C_SendData(reinterpret_cast<I2C_TypeDef*>(mPeripherie), regAddr);

    return true;
}

constexpr const std::array<const I2c, I2c::__ENUM__SIZE> hal::Factory<I2c>::Container;
constexpr const std::array<const uint32_t, I2c::__ENUM__SIZE> hal::Factory<I2c>::Clocks;
std::array<os::Mutex, I2c::Description::__ENUM__SIZE> I2c::MutexArray;
