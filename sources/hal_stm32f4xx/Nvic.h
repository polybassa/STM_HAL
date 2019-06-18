/* Copyright (C) 2018  Henning Mende
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Created on: Nov 9, 2018
 *      Author: Henning Mende
 *
 *  Abstraction for the nested vector interrupt controller.
 */

#ifndef SOURCES_HAL_STM32F4XX_NVIC_H_
#define SOURCES_HAL_STM32F4XX_NVIC_H_

#include "hal_Factory.h"
#include <functional>
#include "stm32f4xx_misc.h"

namespace hal
{
struct Nvic {
#include "Nvic_config.h"

    Nvic() = delete;
    Nvic(const Nvic&) = delete;
    Nvic(Nvic&&) = default;
    Nvic& operator=(const Nvic&) = delete;
    Nvic& operator=(Nvic&&) = delete;

    /**
     * Enables the specific interrupt service routine, but only if the procedures
     * for getting the interrupt status and clearing the interrupt bit are set.
     */
    bool enable(void) const;
    void disable(void) const;

    /**
     * Registers a bool returning void procedure, to determine whether the
     * desired interrupt really happened.
     * @param procedure the checking procedure (bool procedure(void))
     */
    void registerGetInterruptStatusProcedure(std::function<bool(void)> procedure) const;
    /**
     * Registers a void procedure that clears the interrupt status of the
     * hardware.
     */
    void registerClearInterruptProcedure(std::function<void(void)> procedure) const;

    void registerInterruptCallback(std::function<void(void)> callback) const;
    void unregisterInterruptCallback(void) const;
    void handleInterrupt(void) const;

    void setPriority(const uint32_t& prio) const;
    void setPriority(const uint32_t& intPrio, const uint32_t subPrio) const;
    uint32_t getPriority(void) const;

private:
    constexpr Nvic(
                   const enum Description& desc,
                   const IRQn&             interruptName,
                   const uint32_t          interruptPriority = NON_VALID_PRIORITY,
                   const uint32_t          subPriority = NON_VALID_PRIORITY) :
        mDescription(desc),
        mInterruptChannel(interruptName),
        mInitialIntPriority(interruptPriority),
        mInitialSubPriority(subPriority)
    {}

    const enum Description mDescription;
    const IRQn mInterruptChannel;

    using CallbackArray = std::array<std::function<void (void)>, Nvic::__ENUM__SIZE>;
    static CallbackArray NvicCallbacks;

    using GetInterruptStatusProcedureArray = std::array<std::function<bool (void)>, Nvic::__ENUM__SIZE>;
    static GetInterruptStatusProcedureArray NvicGetInterruptStatusProcedeures;

    using ClearInterruptProcedureArray = std::array<std::function<void (void)>, Nvic::__ENUM__SIZE>;
    static ClearInterruptProcedureArray NvicClearInterruptProcedeures;

    void clearInterruptBit(void) const;
    void executeCallback(void) const;
    void initialize(void) const;
    bool getStatus(void) const;

    static constexpr uint32_t NON_VALID_PRIORITY = 0xFFFFFFFF;
    const uint32_t mInitialIntPriority;
    const uint32_t mInitialSubPriority;

    friend class Factory<Nvic>;
};

template<>
class Factory<Nvic>
{
private:
#include "Nvic_config.h"

    Factory(void)
    {
        for (const auto& obj : Container) {
            obj.initialize();
        }
    }

    template<enum IRQn IRQ_Channel, enum Nvic::Description index>
    static constexpr const Nvic& getByIrqChannel(void)
    {
        return (Container[index]).mInterruptChannel ==
               IRQ_Channel ? Container[index] : getByIrqChannel<IRQ_Channel,
                                                                static_cast<enum Nvic::Description>(index - 1)>();
    }

public:

    template<enum Nvic::Description index>
    static constexpr const Nvic& get(void)
    {
        static_assert(Container[index].mInterruptChannel < IRQn::FPU_IRQn, "Interrupt Channel not in IRQ range");
        static_assert(CHECK_INTERRUPT_HANDER_ENABLED(Container[index].mInterruptChannel),
                      "Interrupt handler is not enabled");

        static_assert(index != Nvic::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(index < Container[index + 1].mDescription, "Incorrect order of Nvics in Factory");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<enum IRQn IRQ_Channel>
    static constexpr const Nvic& getByIrqChannel(void)
    {
        static_assert(IRQ_Channel < IRQn::FPU_IRQn, "Interrupt Channel not in IRQ range");

        static_assert(CHECK_INTERRUPT_HANDER_ENABLED(IRQ_Channel),
                      "Interrupt handler is not enabled");

        return getByIrqChannel<IRQ_Channel,
                               static_cast<enum Nvic::Description>(Nvic::Description::__ENUM__SIZE - 1)>();
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_HAL_STM32F4XX_NVIC_H_ */
