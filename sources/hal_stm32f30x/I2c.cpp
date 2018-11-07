// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "I2c.h"
#include "trace.h"
#include "LockGuard.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::I2c;

void I2c::initialize() const
{
    if (!IS_I2C_ALL_PERIPH_BASE(mPeripherie)) {
        return;
    }

    I2C_DeInit(reinterpret_cast<I2C_TypeDef*>(mPeripherie));

    I2C_Init(reinterpret_cast<I2C_TypeDef*>(mPeripherie), &mConfiguration);
    I2C_Cmd(reinterpret_cast<I2C_TypeDef*>(mPeripherie), ENABLE);
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

size_t I2c::write(const uint16_t deviceAddr, const uint8_t regAddr, uint8_t const* const data,
                  const size_t length) const
{
    if (length > 255) {
        Trace(ZONE_ERROR, "Transferlength greater than 255 not implemented, yet!");
        return 0;
    }

    os::LockGuard<os::Mutex> lock(MutexArray[static_cast<size_t>(mDescription)]);

    if (timeoutDuringWaitUntilFlagIsEqualState(I2C_FLAG_BUSY, RESET)) {
        return 0;
    }

    /* Configure slave address, nbytes, reload, end mode and start or stop generation */
    I2C_TransferHandling(reinterpret_cast<I2C_TypeDef*>(mPeripherie),
                         deviceAddr,
                         1,
                         I2C_Reload_Mode,
                         I2C_Generate_Start_Write);

    if (timeoutDuringWaitUntilFlagIsEqualState(I2C_FLAG_TXIS, SET)) {
        return 0;
    }

    I2C_SendData(reinterpret_cast<I2C_TypeDef*>(mPeripherie), (uint8_t)regAddr);

    if (timeoutDuringWaitUntilFlagIsEqualState(I2C_FLAG_TCR, SET)) {
        return 0;
    }

    I2C_TransferHandling(reinterpret_cast<I2C_TypeDef*>(mPeripherie),
                         deviceAddr,
                         length,
                         I2C_AutoEnd_Mode,
                         I2C_No_StartStop);

    size_t bytesSend = 0;
    while (bytesSend < length) {
        if (timeoutDuringWaitUntilFlagIsEqualState(I2C_FLAG_TXIS, SET)) {
            return bytesSend;
        }
        /* Configure slave address, nbytes, reload, end mode and start or stop generation */
        I2C_SendData(reinterpret_cast<I2C_TypeDef*>(mPeripherie), data[bytesSend]);
        bytesSend++;
    }

    if (timeoutDuringWaitUntilFlagIsEqualState(I2C_FLAG_STOPF, SET)) {
        return bytesSend;
    }

    I2C_ClearFlag(reinterpret_cast<I2C_TypeDef*>(mPeripherie), I2C_FLAG_STOPF);
    return bytesSend;
}

size_t I2c::read(const uint16_t deviceAddr, const uint8_t regAddr, uint8_t* const data, const size_t length) const
{
    if (length > 255) {
        Trace(ZONE_ERROR, "Transferlength greater than 255 not implemented, yet!");
        return 0;
    }

    os::LockGuard<os::Mutex> lock(MutexArray[static_cast<size_t>(mDescription)]);

    if (timeoutDuringWaitUntilFlagIsEqualState(I2C_FLAG_BUSY, RESET)) {
        return 0;
    }

    /* Configure slave address, nbytes, reload, end mode and start or stop generation */
    I2C_TransferHandling(reinterpret_cast<I2C_TypeDef*>(mPeripherie),
                         deviceAddr,
                         1,
                         I2C_SoftEnd_Mode,
                         I2C_Generate_Start_Write);

    if (timeoutDuringWaitUntilFlagIsEqualState(I2C_FLAG_TXIS, SET)) {
        return 0;
    }

    I2C_SendData(reinterpret_cast<I2C_TypeDef*>(mPeripherie), regAddr);
    if (timeoutDuringWaitUntilFlagIsEqualState(I2C_FLAG_TC, SET)) {
        return 0;
    }

    /* Configure slave address, nbytes, reload, end mode and start or stop generation */
    I2C_TransferHandling(reinterpret_cast<I2C_TypeDef*>(mPeripherie),
                         deviceAddr,
                         length,
                         I2C_AutoEnd_Mode,
                         I2C_Generate_Start_Read);

    size_t bytesReceived = 0;
    while (bytesReceived < length) {
        if (timeoutDuringWaitUntilFlagIsEqualState(I2C_FLAG_RXNE, SET)) {
            return bytesReceived;
        }
        data[bytesReceived] = I2C_ReceiveData(reinterpret_cast<I2C_TypeDef*>(mPeripherie));

        bytesReceived++;
    }

    if (timeoutDuringWaitUntilFlagIsEqualState(I2C_FLAG_STOPF, SET)) {
        return bytesReceived;
    }

    I2C_ClearFlag(reinterpret_cast<I2C_TypeDef*>(mPeripherie), I2C_ICR_STOPCF);

    return bytesReceived;
}

constexpr const std::array<const I2c, I2c::__ENUM__SIZE> hal::Factory<I2c>::Container;
constexpr const std::array<const uint32_t, I2c::__ENUM__SIZE> hal::Factory<I2c>::Clocks;
std::array<os::Mutex, I2c::Description::__ENUM__SIZE> I2c::MutexArray;
