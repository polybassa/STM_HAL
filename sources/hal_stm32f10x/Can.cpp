// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "Can.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

using hal::Can;
using hal::Factory;

#if USB_LP_CAN1_RX0_INTERRUPT_ENABLED
void    USB_LP_CAN1_RX0_IRQHandler(void)
{
    constexpr const Can& can = Factory<Can>::get<Can::Description::MAINCAN>();
    Can::Can_IRQHandler(can);
}
#endif

#if CAN1_RX1_INTERRUPT_ENABLED
void    CAN1_RX1_IRQHandler(void)
{
    constexpr const Can& can = Factory<Can>::get<Can::Description::MAINCAN>();
    Can::Can_IRQHandler(can);
}
#endif

void Can::Can_IRQHandler(const Can& peripherie)
{
    static CanRxMsg msg;

    if (CAN_GetITStatus(reinterpret_cast<CAN_TypeDef*>(peripherie.mPeripherie), CAN_IT_FMP0)) {
        if (Can::ReceiveInterruptCallbacks[peripherie.mDescription]) {
            CAN_Receive(reinterpret_cast<CAN_TypeDef*>(peripherie.mPeripherie), CAN_FIFO0, &msg);
            Can::ReceiveInterruptCallbacks[peripherie.mDescription](msg);
        }
        CAN_ClearITPendingBit(reinterpret_cast<CAN_TypeDef*>(peripherie.mPeripherie), CAN_IT_FMP0);
    }
    if (CAN_GetITStatus(reinterpret_cast<CAN_TypeDef*>(peripherie.mPeripherie), CAN_IT_FMP1)) {
        if (Can::ReceiveInterruptCallbacks[peripherie.mDescription]) {
            CAN_Receive(reinterpret_cast<CAN_TypeDef*>(peripherie.mPeripherie), CAN_FIFO1, &msg);
            Can::ReceiveInterruptCallbacks[peripherie.mDescription](msg);
        }
        CAN_ClearITPendingBit(reinterpret_cast<CAN_TypeDef*>(peripherie.mPeripherie), CAN_IT_FMP1);
    }
}

void Can::initialize() const
{
    CAN_DeInit(reinterpret_cast<CAN_TypeDef*>(mPeripherie));
    CAN_Init(reinterpret_cast<CAN_TypeDef*>(mPeripherie), &mConfiguration);

    CAN_FilterInitTypeDef CAN_FilterInitStructure;
    /* CAN filter init */
    CAN_FilterInitStructure.CAN_FilterNumber = 0;
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0x0700 << 5;
    CAN_FilterInitStructure.CAN_FilterIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0x07F0 << 5;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0x0000;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = 0;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);

    // Initialize Interrupts
    NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0xf);
    NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
    NVIC_SetPriority(CAN1_RX1_IRQn, 0xf);
    NVIC_EnableIRQ(CAN1_RX1_IRQn);
}

void Can::enableNonBlockingReceive(std::function<void(CanRxMsg)> callback) const
{
    ReceiveInterruptCallbacks[mDescription] = callback;

    CAN_ClearITPendingBit(reinterpret_cast<CAN_TypeDef*>(mPeripherie), CAN_IT_FMP0);
    CAN_ClearITPendingBit(reinterpret_cast<CAN_TypeDef*>(mPeripherie), CAN_IT_FMP1);

    CAN_ITConfig(reinterpret_cast<CAN_TypeDef*>(mPeripherie), CAN_IT_FMP0 | CAN_IT_FMP1, ENABLE);
}

void Can::disableNonBlockingReceive(void) const
{
    CAN_ITConfig(reinterpret_cast<CAN_TypeDef*>(mPeripherie), CAN_IT_FMP0 | CAN_IT_FMP1, DISABLE);
}

bool Can::send(CanTxMsg& msg) const
{
    auto ret = CAN_Transmit(reinterpret_cast<CAN_TypeDef*>(mPeripherie), &msg);

    return ret != CAN_TxStatus_NoMailBox;
}

size_t Can::messagePending(void) const
{
    size_t ret = CAN_MessagePending(reinterpret_cast<CAN_TypeDef*>(mPeripherie), CAN_FIFO0);
    ret += CAN_MessagePending(reinterpret_cast<CAN_TypeDef*>(mPeripherie), CAN_FIFO1);
    return ret;
}

bool Can::receive(CanRxMsg& msg) const
{
    if (CAN_MessagePending(reinterpret_cast<CAN_TypeDef*>(mPeripherie), CAN_FIFO0)) {
        CAN_Receive(reinterpret_cast<CAN_TypeDef*>(mPeripherie), CAN_FIFO0, &msg);
        return true;
    } else if (CAN_MessagePending(reinterpret_cast<CAN_TypeDef*>(mPeripherie), CAN_FIFO1)) {
        CAN_Receive(reinterpret_cast<CAN_TypeDef*>(mPeripherie), CAN_FIFO1, &msg);
        return true;
    }
    return false;
}

bool Can::hasOverRunError(void) const
{
    return CAN_GetFlagStatus(reinterpret_cast<CAN_TypeDef*>(mPeripherie),
                             CAN_FLAG_FOV1 | CAN_FLAG_FOV0) == SET ? true : false;
}

void Can::clearOverRunError(void) const
{
    CAN_ClearFlag(reinterpret_cast<CAN_TypeDef*>(mPeripherie), CAN_FLAG_FOV1 | CAN_FLAG_FOV0);
}

Can::ReceiveCallbackArray Can::ReceiveInterruptCallbacks;

constexpr const std::array<const Can, Can::__ENUM__SIZE + 1> Factory<Can>::Container;
constexpr const std::array<const CAN_FilterInitTypeDef, 1> Factory<Can>::CanFilterContainer;
