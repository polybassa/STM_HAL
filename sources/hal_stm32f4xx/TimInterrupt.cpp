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

    mNvic.registerGetInterruptStatusProcedure([timBasePointer, interruptSource]() -> bool {
        return TIM_GetITStatus(timBasePointer, interruptSource) != RESET;
    });

    mNvic.registerClearInterruptProcedure([timBasePointer, interruptSource](){
        TIM_ClearITPendingBit(timBasePointer, interruptSource);
    });

    TIM_ClearITPendingBit(timBasePointer, interruptSource);
    mNvic.enable();
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

    if (IS_TIM_GET_IT(tim_it)) {
        return tim_it;
    } else {
        return TIM_IT_Update;
    }
}

void hal::TimInterrupt::setCompareValue(const uint32_t& compare) const
{
    switch (mInterruptSource) {
    case TIM_IT_CC1:
        TIM_SetCompare1(mTim.getBasePointer(), compare);
        break;

    case TIM_IT_CC2:
        TIM_SetCompare2(mTim.getBasePointer(), compare);
        break;

    case TIM_IT_CC3:
        TIM_SetCompare3(mTim.getBasePointer(), compare);
        break;

    case TIM_IT_CC4:
        TIM_SetCompare4(mTim.getBasePointer(), compare);
        break;

    default:
        // nothing to be done
        break;
    }
}

const uint32_t hal::TimInterrupt::getCompareValue(void) const
{
    uint32_t compVal = 0;

    switch (mInterruptSource) {
    case TIM_IT_CC1:
        compVal = mTim.getBasePointer()->CCR1;
        break;

    case TIM_IT_CC2:
        compVal = mTim.getBasePointer()->CCR1;
        break;

    case TIM_IT_CC3:
        compVal = mTim.getBasePointer()->CCR1;
        break;

    case TIM_IT_CC4:
        compVal = mTim.getBasePointer()->CCR1;
        break;

    default:
        compVal = 0;
        break;
    }

    return compVal;
}
