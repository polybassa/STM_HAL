/*
 * TimInt.cpp
 *
 *  Created on: Nov 9, 2018
 *      Author: pmd
 */

#include "TimInterrupt.h"

namespace hal
{} /* namespace hal */

hal::TimInterrupt::TimInterrupt(const Tim& tim, const uint16_t& tim_it, const Nvic& nvic) :
    mTim(tim),
    mInterruptSource(checkInterruptSource(tim_it)),
    mNvic(nvic)
{
    TIM_TypeDef* timBasePointer = mTim.getBasePointer();
    const uint16_t interruptSource = mInterruptSource;

    mNvic.registerGetInterruptStatusProcedure([timBasePointer, interruptSource]()->bool {
                                                  return TIM_GetITStatus(timBasePointer, interruptSource) != RESET;
                                              });

    mNvic.registerClearInterruptProcedure([timBasePointer, interruptSource](){
                                              TIM_ClearITPendingBit(timBasePointer, interruptSource);
                                          });

    TIM_ClearITPendingBit(timBasePointer, interruptSource);
    mNvic.enable(); //TODO check for error status
}

void hal::TimInterrupt::enable(void) const
{
    TIM_ITConfig(mTim.getBasePointer(), mInterruptSource, ENABLE);
}

void hal::TimInterrupt::disable(void) const
{
    TIM_ITConfig(mTim.getBasePointer(), mInterruptSource, DISABLE);
}

void hal::TimInterrupt::registerInterruptCallback(
                                                  std::function<void(void)> function) const
{
    mNvic.registerInterruptCallback(function);
}

void hal::TimInterrupt::unregisterInterruptCallback(void) const
{
    mNvic.unregisterInterruptCallback();
}

void hal::TimInterrupt::handleInterrupt(void) const
{
    mNvic.handleInterrupt();
}

const uint16_t hal::TimInterrupt::checkInterruptSource(const uint16_t& tim_it) const
{
    //	static_assert(IS_TIM_IT(mInterruptSource), "Invalid interrupt source!");

    if (IS_TIM_IT(tim_it)) {
        return tim_it;
    } else {
        return TIM_IT_Update;
    }
}
