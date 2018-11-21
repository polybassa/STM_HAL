// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 * Implementation for stm32f10x medium density line
 */
#pragma once

#include <cstdint>
#include <limits>
#include <array>
#include <functional>
#include "stm32f10x_can.h"
#include "stm32f10x_rcc.h"
#include "hal_Factory.h"

extern "C" {
void    USB_LP_CAN1_RX0_IRQHandler(void);
void    CAN1_RX1_IRQHandler(void);
}

namespace hal
{
struct Can {
#include "Can_config.h"

    const enum Description mDescription;

    Can() = delete;
    Can(const Can&) = delete;
    Can(Can&&) = default;
    Can& operator=(const Can&) = delete;
    Can& operator=(Can&&) = delete;

    bool hasOverRunError(void) const;
    void clearOverRunError(void) const;

    bool send(CanTxMsg&) const;
    size_t messagePending(void) const;
    bool receive(CanRxMsg& msg) const;

    void enableNonBlockingReceive(std::function<void(CanRxMsg)> callback) const;
    void disableNonBlockingReceive(void) const;

    static void Can_IRQHandler(const Can& peripherie);

private:
    constexpr Can(const enum Description& desc,
                  const uint32_t&         peripherie,
                  const CAN_InitTypeDef&  conf) :
        mDescription(desc),
        mPeripherie(peripherie),
        mConfiguration(conf) {}

    const uint32_t mPeripherie;
    const CAN_InitTypeDef mConfiguration;

    void initialize(void) const;

    using ReceiveCallbackArray = std::array<std::function<void (CanRxMsg)>, Can::__ENUM__SIZE>;
    static ReceiveCallbackArray ReceiveInterruptCallbacks;

    friend class Factory<Can>;
};

template<>
class Factory<Can>
{
#include "Can_config.h"

    Factory(void)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);
        Container[0].initialize();
    }

public:
    template<enum Can::Description index>
    static constexpr const Can& get(void)
    {
        static_assert(IS_CAN_ALL_PERIPH_BASE(Container[index].mPeripherie), "Invalid Peripheries ");
        static_assert(IS_CAN_MODE(Container[index].mConfiguration.CAN_Mode), "Invalid Parameter");
        static_assert(IS_CAN_SJW(Container[index].mConfiguration.CAN_SJW), "Invalid Parameter");
        static_assert(IS_CAN_BS1(Container[index].mConfiguration.CAN_BS1), "Invalid Parameter");
        static_assert(IS_CAN_BS2(Container[index].mConfiguration.CAN_BS2), "Invalid Parameter ");
        static_assert(IS_CAN_PRESCALER(Container[index].mConfiguration.CAN_Prescaler), "Invalid Parameter");

        static_assert(index != Can::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(index < Container[index + 1].mDescription, "Incorrect order of instances in Factory");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}
