/**
 * LedBlinker.h
 *
 * first app test.
 * Cycles the four LEDs on the STM32F4VG-Discovery Board.
 *
 * *  Created on: Aug 21, 2018
 *      Author: Henning Mende
 */

#ifndef SOURCES_LED_BLINKER_H_
#define SOURCES_LED_BLINKER_H_

#include "Gpio.h"
#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include <chrono>

namespace app
{
class LedBlinker final :
    private os::DeepSleepModule
{
public:
    LedBlinker(int frequency, std::chrono::milliseconds initialWaitTime = std::chrono::milliseconds(0));

    LedBlinker(const LedBlinker&) = delete;
    LedBlinker(LedBlinker&&) = delete;
    LedBlinker& operator=(const LedBlinker&) = delete;
    LedBlinker& operator=(LedBlinker&&) = delete;

    void changeDirection(void);
    void setDirectionClockwise(const bool& clockwise);

    void yield(void)
    {
        this->enterDeepSleep();
    }

    void resume(void)
    {
        this->exitDeepSleep();
    }

private:
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr uint32_t STACKSIZE = 400;
    os::TaskInterruptable mLedBlinkerTask;
    const std::chrono::milliseconds mControllerInterval;
    const std::chrono::milliseconds mInitialWaitTime;

    void cycleLights(const bool&);

    inline int8_t prevLedNumber(const int8_t&) const;
    inline int8_t nextLedNumber(const int8_t&) const;
    inline int8_t limitLedNumber(const int8_t&) const;

    bool mClockwise = true;
};
}
#endif /* SOURCES_LED_BLINKER_H_ */
