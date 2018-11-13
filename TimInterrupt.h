/*
 * TimInt.h
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

private:

    const Tim& mTim;
    const uint16_t mInterruptSource;
    const Nvic& mNvic;

    const uint16_t checkInterruptSource(const uint16_t& tim_it) const;
};
}

#endif /* PROJECT_CONFIG_TIMINT_H_ */
