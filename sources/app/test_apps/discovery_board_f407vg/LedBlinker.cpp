/*
 * LedBlinker.cpp
 *
 * first app test.
 * Cycles the four LEDs on the STM32F4VG-Discovery Board.
 *
 *  Created on: Aug 21, 2018
 *      Author: Henning Mende
 */

#include "discovery_board_f407vg/LedBlinker.h"
#include "Exti.h"

app::LedBlinker::LedBlinker(int frequency, std::chrono::milliseconds initialWaitTime) :
    mLedBlinkerTask(
                    "LedBlinker",
                    LedBlinker::STACKSIZE,
                    os::Task::Priority::LOW,
                    [this](const bool& join){
    cycleLights(join);
}),
    mControllerInterval(std::chrono::milliseconds(250 / frequency)),
    mInitialWaitTime(initialWaitTime)
{}

void app::LedBlinker::changeDirection(void)
{
    mClockwise = !mClockwise;
}

void app::LedBlinker::setDirectionClockwise(const bool& clockwise)
{
    mClockwise = clockwise;
}

void app::LedBlinker::enterDeepSleep(void)
{
    mLedBlinkerTask.join();
}

void app::LedBlinker::exitDeepSleep(void)
{
    mLedBlinkerTask.start();
}

void app::LedBlinker::cycleLights(const bool& join)
{
    const hal::Gpio& led_green = hal::Factory<hal::Gpio>::get<hal::Gpio::LED_GREEN>();
    const hal::Gpio& led_orange = hal::Factory<hal::Gpio>::get<hal::Gpio::LED_ORANGE>();
    const hal::Gpio& led_blue = hal::Factory<hal::Gpio>::get<hal::Gpio::LED_BLUE>();
    const hal::Gpio& led_red = hal::Factory<hal::Gpio>::get<hal::Gpio::LED_RED>();

    const hal::Gpio* led_array[4] = {&led_green, &led_orange, &led_red, &led_blue};

    int8_t currentLed = prevLedNumber(0);

    os::ThisTask::sleep(mInitialWaitTime);

    do {
        *(led_array[currentLed]) = false;

        currentLed = nextLedNumber(currentLed);

        *(led_array[currentLed]) = true;

        os::ThisTask::sleep(mControllerInterval);
    } while (!join);
}

int8_t app::LedBlinker::prevLedNumber(const int8_t& currentLedNumber) const
{
    int8_t newValue = 0;

    if (mClockwise) {
        newValue = currentLedNumber - 1;
    } else {
        newValue = currentLedNumber + 1;
    }

    return limitLedNumber(newValue);
}

int8_t app::LedBlinker::nextLedNumber(const int8_t& currentLedNumber) const
{
    int8_t newValue = 0;

    if (mClockwise) {
        newValue = currentLedNumber + 1;
    } else {
        newValue = currentLedNumber - 1;
    }

    return limitLedNumber(newValue);
}

int8_t app::LedBlinker::limitLedNumber(const int8_t& currentLedNumber) const
{
    int8_t limitedValue = 0;

    if (currentLedNumber > 3) {
        limitedValue = 0;
    } else if (currentLedNumber < 0) {
        limitedValue = 3;
    } else {
        limitedValue = currentLedNumber;
    }

    return limitedValue;
}
