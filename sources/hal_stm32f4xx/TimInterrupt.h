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
 *      abstaction for timer interrupts (additional to hal::Tim, which doesn't support intterupts)
 */

#ifndef PROJECT_CONFIG_TIMINT_H_
#define PROJECT_CONFIG_TIMINT_H_

#include "Tim.h"
#include "Nvic.h"

namespace hal
{
class TimInterrupt
{
public:

    TimInterrupt(const Tim& tim, const uint16_t& tim_it, const Nvic& nvic);

    void enable(void) const;
    void disable(void) const;
    void registerInterruptCallback(std::function<void(void)> ) const;
    void unregisterInterruptCallback(void) const;
    void handleInterrupt(void) const;

    /**
     * Sets the compare value if a capture compare event is configured as interrupt source.
     * If the interrupt source is not a capture compare event, this has no effect.
     * @param compare the value for the capture compare register of the timer.
     */
    void setCompareValue(const uint32_t& compare) const;
    /**
     * Returns the corresponding compare value. Zero if no capture compare event is set as interrupt source.
     */
    const uint32_t getCompareValue(void) const;

private:

    const Tim& mTim;
    const uint16_t mInterruptSource;
    const Nvic& mNvic;

    const uint16_t checkInterruptSource(const uint16_t& tim_it) const;
};
}

#endif /* PROJECT_CONFIG_TIMINT_H_ */
