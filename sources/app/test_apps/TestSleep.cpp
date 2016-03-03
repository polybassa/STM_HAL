/* Copyright (C) 2015  Nils Weiss
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "TestSleep.h"
#include "trace.h"
#include "Light.h"
#include <chrono>
#include "DeepSleepInterface.h"
#include "TaskInterruptable.h"
#include "Task.h"
#include "Gpio.h"
#include "Exti.h"
#include "stm32f30x_pwr.h"
#include "FreeRTOS.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

class BlinkApp : os::DeepSleepModule {
private:
    os::TaskInterruptable mBlinkTask;

    virtual void enterDeepSleep(void) override
    {
        mBlinkTask.join();
    }
    virtual void exitDeepSleep(void) override
    {
        mBlinkTask.start();
    }

    void blinkAppTaskFunction(const bool&);

public:
    BlinkApp(void) :
        os::DeepSleepModule(),
            mBlinkTask("blinky", 2048, os::Task::Priority::LOW, [this](const bool& join){
        blinkAppTaskFunction(join);
    }){}

    BlinkApp(const BlinkApp&) = delete;
    BlinkApp(BlinkApp&&) = delete;
    BlinkApp& operator=(const BlinkApp&) = delete;
    BlinkApp& operator=(BlinkApp&&) = delete;
};

void BlinkApp::blinkAppTaskFunction(const bool& join)
{
    using hal::Factory;
    using hal::Gpio;
    using namespace std::chrono_literals;

    static constexpr const auto& led0 = Factory<Gpio>::get<Gpio::LED_3>();
    static constexpr const auto& led1 = Factory<Gpio>::get<Gpio::LED_4>();

    led0 = true;
    led1 = true;
    do {
        os::ThisTask::sleep(100ms);
        led0 = true;
        led1 = false;
        os::ThisTask::sleep(100ms);
        led0 = !true;
        led1 = !false;
    } while (!join);

    led0 = false;
    led1 = false;
}

BlinkApp blink;

static void SYSCLKConfig_STOP(void)
{
    /* After wake-up from STOP reconfigure the system clock */
    /* Enable HSE */
    RCC_HSEConfig(RCC_HSE_ON);

    /* Wait till HSE is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET) {}

    /* Enable PLL */
    RCC_PLLCmd(ENABLE);

    /* Wait till PLL is ready */
    while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}

    /* Select PLL as system clock source */
    RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

    /* Wait till PLL is used as system clock source */
    while (RCC_GetSYSCLKSource() != 0x08) {}
}

os::TaskEndless sleepTest("Sleep_Test", 2048, os::Task::Priority::MEDIUM, [](const bool&) {
    using hal::Exti;
    using hal::Factory;
    using hal::Gpio;
    static constexpr const auto& button = Factory<Gpio>::get<Gpio::USER_BUTTON>();
    static constexpr const auto& wakeup = Factory<Exti>::get<Exti::WAKEUP>();
    static constexpr const auto& led2 = Factory<Gpio>::get<Gpio::LED_5>();

    wakeup.enable();

    while (true) {
        if (!button) {
            os::ThisTask::sleep(std::chrono::milliseconds(20));
            continue;
        }

        while (button) {
            os::ThisTask::sleep(std::chrono::milliseconds(20));
        }

        os::ThisTask::sleep(std::chrono::milliseconds(50));

        os::DeepSleepController::enterGlobalDeepSleep();

        os::Task::suspendAll();

        led2 = false;

        PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

        portDISABLE_INTERRUPTS();
        SYSCLKConfig_STOP();
        SysTick_Config((configCPU_CLOCK_HZ / configTICK_RATE_HZ) - 1UL);
        portENABLE_INTERRUPTS();

        led2 = true;

        os::Task::resumeAll();

        os::DeepSleepController::exitGlobalDeepSleep();
        os::ThisTask::sleep(std::chrono::milliseconds(500));
    }
});
