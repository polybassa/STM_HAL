// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "Usart.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Factory;
using hal::Usart;

#if USART1_INTERRUPT_ENABLED
void USART1_IRQHandler(void)
{
    constexpr const Usart& uart = Factory<Usart>::getByPeripherie<USART1_BASE>();
    Usart::USART_IRQHandler(uart);
}
#endif

#if USART2_INTERRUPT_ENABLED
void USART2_IRQHandler(void)
{
    constexpr const Usart& uart = Factory<Usart>::getByPeripherie<USART2_BASE>();
    Usart::USART_IRQHandler(uart);
}
#endif

#if USART3_INTERRUPT_ENABLED
void USART3_IRQHandler(void)
{
    constexpr const Usart& uart = Factory<Usart>::getByPeripherie<USART3_BASE>();
    Usart::USART_IRQHandler(uart);
}
#endif

#if USART4_INTERRUPT_ENABLED
void UART4_IRQHandler(void)
{
    constexpr const Usart& uart = Factory<Usart>::getByPeripherie<UART4_BASE>();
    Usart::USART_IRQHandler(uart);
}
#endif

#if USART5_INTERRUPT_ENABLED
void UART5_IRQHandler(void)
{
    constexpr const Usart& uart = Factory<Usart>::getByPeripherie<UART5_BASE>();
    Usart::USART_IRQHandler(uart);
}
#endif

void Usart::USART_IRQHandler(const Usart& peripherie)
{
    if (USART_GetITStatus(reinterpret_cast<USART_TypeDef*>(peripherie.mPeripherie), USART_IT_RXNE)) {
        if (Usart::ReceiveInterruptCallbacks[peripherie.mDescription]) {
            uint8_t databyte =
                static_cast<uint8_t>(USART_ReceiveData(reinterpret_cast<USART_TypeDef*>(peripherie.mPeripherie)));
            Usart::ReceiveInterruptCallbacks[peripherie.mDescription](databyte);
        }
        USART_ClearITPendingBit(reinterpret_cast<USART_TypeDef*>(peripherie.mPeripherie), USART_IT_RXNE);
    }
}

void Usart::initialize() const
{
    USART_DeInit(reinterpret_cast<USART_TypeDef*>(mPeripherie));
    USART_Init(reinterpret_cast<USART_TypeDef*>(mPeripherie), &mConfiguration);

    USART_Cmd(reinterpret_cast<USART_TypeDef*>(mPeripherie), ENABLE);
    mInitalized = true;

    // Initialize Interrupts
    NVIC_SetPriority(getIRQn(), 0xf);
    NVIC_EnableIRQ(getIRQn());
}

IRQn Usart::getIRQn(void) const
{
    switch (mPeripherie) {
    case USART1_BASE:
        return IRQn::USART1_IRQn;

    case USART2_BASE:
        return IRQn::USART2_IRQn;

    case USART3_BASE:
        return IRQn::USART3_IRQn;

    case UART4_BASE:
        return IRQn::UART4_IRQn;

    case UART5_BASE:
        return IRQn::UART5_IRQn;
    }

    return IRQn::UsageFault_IRQn;
}

bool Usart::isInitalized(void) const
{
    return mInitalized;
}

void Usart::setBaudRate(const size_t baudRate) const
{
    USART_InitTypeDef configuration = mConfiguration;
    configuration.USART_BaudRate = baudRate;

    USART_Init(reinterpret_cast<USART_TypeDef*>(mPeripherie), &configuration);
    USART_Cmd(reinterpret_cast<USART_TypeDef*>(mPeripherie), ENABLE);
}

void Usart::enableNonBlockingReceive(std::function<void(uint8_t)> callback) const
{
    ReceiveInterruptCallbacks[mDescription] = callback;

    USART_ClearITPendingBit(reinterpret_cast<USART_TypeDef*>(mPeripherie), USART_IT_RXNE);
    USART_ITConfig(reinterpret_cast<USART_TypeDef*>(mPeripherie), USART_IT_RXNE, ENABLE);
}

void Usart::disableNonBlockingReceive(void) const
{
    ReceiveInterruptCallbacks[mDescription] = nullptr;

    USART_ITConfig(reinterpret_cast<USART_TypeDef*>(mPeripherie), USART_IT_RXNE, DISABLE);
}

void Usart::send(const uint16_t data) const
{
    USART_SendData(reinterpret_cast<USART_TypeDef*>(mPeripherie), data);
}

size_t Usart::send(uint8_t const* const data, const size_t length) const
{
    if (data == nullptr) {
        return 0;
    }

    size_t bytesSend = 0;
    while (bytesSend < length) {
        if (this->isReadyToSend()) {
            this->send(data[bytesSend]);
            bytesSend++;
        }
    }
    return bytesSend;
}

bool Usart::isReadyToReceive(void) const
{
    return USART_GetFlagStatus(reinterpret_cast<USART_TypeDef*>(mPeripherie), USART_FLAG_RXNE);
}

uint16_t Usart::receive(void) const
{
    return USART_ReceiveData(reinterpret_cast<USART_TypeDef*>(mPeripherie));
}

size_t Usart::receive(uint8_t* const data, const size_t length) const
{
    if (data == nullptr) {
        return 0;
    }

    size_t bytesReceived = 0;
    while (bytesReceived < length) {
        if (this->isReadyToReceive()) {
            data[bytesReceived] = (uint8_t)this->receive();
            bytesReceived++;
        }
    }
    return bytesReceived;
}

size_t Usart::receiveAvailableData(uint8_t* const data, const size_t length) const
{
    if (data == nullptr) {
        return 0;
    }

    size_t bytesReceived = 0;
    while (bytesReceived < length) {
        if (this->isReadyToReceive()) {
            data[bytesReceived] = (uint8_t)receive();
            bytesReceived++;
        } else {
            return bytesReceived;
        }
    }
    return bytesReceived;
}

bool Usart::isReadyToSend(void) const
{
    return USART_GetFlagStatus(reinterpret_cast<USART_TypeDef*>(mPeripherie), USART_FLAG_TXE) == SET ? true : false;
}

bool Usart::hasOverRunError(void) const
{
    return USART_GetFlagStatus(reinterpret_cast<USART_TypeDef*>(mPeripherie), USART_FLAG_ORE) == SET ? true : false;
}

bool Usart::hasNoiseError(void) const
{
    return USART_GetFlagStatus(reinterpret_cast<USART_TypeDef*>(mPeripherie), USART_FLAG_NE) == SET ? true : false;
}

bool Usart::hasFramingError(void) const
{
    return USART_GetFlagStatus(reinterpret_cast<USART_TypeDef*>(mPeripherie), USART_FLAG_FE) == SET ? true : false;
}

bool Usart::hasParityError(void) const
{
    return USART_GetFlagStatus(reinterpret_cast<USART_TypeDef*>(mPeripherie), USART_FLAG_PE) == SET ? true : false;
}

void Usart::clearOverRunError(void) const
{
    USART_ClearFlag(reinterpret_cast<USART_TypeDef*>(mPeripherie), USART_FLAG_ORE);
}

void Usart::clearNoiseError(void) const
{
    USART_ClearFlag(reinterpret_cast<USART_TypeDef*>(mPeripherie), USART_FLAG_NE);
}

void Usart::clearFramingError(void) const
{
    USART_ClearFlag(reinterpret_cast<USART_TypeDef*>(mPeripherie), USART_FLAG_FE);
}

void Usart::clearParityError(void) const
{
    USART_ClearFlag(reinterpret_cast<USART_TypeDef*>(mPeripherie), USART_FLAG_PE);
}

Usart::ReceiveCallbackArray Usart::ReceiveInterruptCallbacks;

constexpr const std::array<const Usart, Usart::__ENUM__SIZE + 1> Factory<Usart>::Container;
constexpr const std::array<const uint32_t, Usart::__ENUM__SIZE> Factory<Usart>::Clocks;
