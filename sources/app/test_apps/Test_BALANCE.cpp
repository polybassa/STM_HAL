/* Copyright (C) 2016 Nils Weiss */

#include "Test_BALANCE.h"
#include "Mpu.h"
#include "RealTimeDebugInterface.h"
#include "Gpio.h"
#include "Adc.h"
#include "AdcChannel.h"

extern app::Mpu* g_mpu;
extern dev::RealTimeDebugInterface* g_RTTerminal;

const os::TaskEndless app::balanceTest("PMD_Demo", 4096, os::Task::Priority::MEDIUM, [&](
                                                                                         const bool&)
                                       {
                                           // Get any GPIO you want. You get it by their name (DESCRIPTION)
                                           constexpr const hal::Gpio& led10 =
                                               hal::Factory<hal::Gpio>::get<hal::Gpio::LED_10>();
                                           constexpr const hal::Gpio& led3 =
                                               hal::Factory<hal::Gpio>::get<hal::Gpio::LED_3>();

                                           constexpr const hal::Adc::Channel& poti1 =
                                               hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::NTC_BATTERY>();
                                           constexpr const hal::Adc::Channel& poti2 =
                                               hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::NTC_FET>();
                                           constexpr const hal::Adc::Channel& poti3 =
                                               hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::NTC_MOTOR>();

                                           while (1) {
                                               Eigen::Vector3f accel = g_mpu->getAcceleration();

                                               os::ThisTask::sleep(std::chrono::milliseconds(5));
                                           }
                                       });
