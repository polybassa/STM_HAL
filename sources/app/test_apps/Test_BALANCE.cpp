/* Copyright (C) 2016 Nils Weiss */

#include "Test_BALANCE.h"
#include "Mpu.h"
#include "Gpio.h"
#include "Adc.h"
#include "AdcChannel.h"
#include "MotorController.h"
#include "PIDController.h"
#include "trace.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

extern app::Mpu* g_Mpu;
extern app::MotorController* g_motorCtrl;

// Get any GPIO you want. You can access them by their name (DESCRIPTION)
//constexpr const hal::Gpio& led10 = hal::Factory<hal::Gpio>::get<hal::Gpio::LED_10>();
//constexpr const hal::Gpio& led3 = hal::Factory<hal::Gpio>::get<hal::Gpio::LED_3>();

constexpr const hal::Adc::Channel& poti1 = hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::NTC_BATTERY>();
constexpr const hal::Adc::Channel& poti2 = hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::NTC_FET>();
constexpr const hal::Adc::Channel& poti3 = hal::Factory<hal::Adc::Channel>::get<hal::Adc::Channel::NTC_MOTOR>();

// Lets prepare a PID Controller for later
float setValue1, currentValue1, outValue1;
const float KP = 0.1;
const float KI = 0.1;
const float KD = 0.1;
dev::PIDController pid1(setValue1,
                        currentValue1,
                        outValue1,
                        KP,
                        KI,
                        KD,
                        dev::PIDController::ControlDirection::DIRECT);

void setup(void)
{
    Trace(ZONE_INFO, "Hi Jakob. Here is some setup code\n");
    Trace(ZONE_INFO, "Turn the motor off, first\n");
    g_motorCtrl->setTorque(0.0);

    Trace(ZONE_INFO, "Wait some ms\n");
    os::ThisTask::sleep(std::chrono::milliseconds(5));

    Trace(ZONE_INFO, "Wait some s\n");
    os::ThisTask::sleep(std::chrono::seconds(5));

//    Trace(ZONE_INFO, "Let some LEDs blink\n");
//    led10 = true;
//    led3 = false;
//    os::ThisTask::sleep(std::chrono::milliseconds(500));
//    led10 = false;
//    led3 = true;
//    os::ThisTask::sleep(std::chrono::milliseconds(500));
//    led10 = true;
//    led3 = true;
//    os::ThisTask::sleep(std::chrono::milliseconds(500));
//    led10 = false;
//    led3 = false;

    float voltagePoti1 = poti1.getVoltage();
    uint32_t valuePoti1 = poti1.getValue();

    Trace(ZONE_INFO, "Poti1 measures %d mV. This is an absolute value of %d\n",
          static_cast<uint32_t>(voltagePoti1 * 1000), valuePoti1);
    Trace(ZONE_INFO, "Unfortunately, this trace interface doesn't support floats.\n");
    Trace(ZONE_INFO, "If you want to print a float, you have to cast it as signed or unsigned value.\n");

    Trace(ZONE_INFO, "\nLets try to read from the MPU6050.\n");

    Eigen::Vector3f gravity = g_Mpu->getGravity();
    Trace(ZONE_INFO, "Gravity: x:%6d, y:%6d, z:%6d\n",
          static_cast<int32_t>(gravity.x() * 1000),
          static_cast<int32_t>(gravity.y() * 1000),
          static_cast<int32_t>(gravity.z() * 1000));

    //setup PID Controller. It needs the sample time of our loop function
    pid1.setSampleTime(std::chrono::milliseconds(5));
    pid1.setOutputLimits(-1.0, 1.0);
}

// This function will run forever
void loop(void)
{
    Eigen::Vector3f accel = g_Mpu->getAcceleration();

    // update the current Value of the PID Controller
    currentValue1 = accel.x();

    // update setValue from Poti
    setValue1 = poti1.getVoltage();

    // PID... do your job
    pid1.compute();

    //update torque
    g_motorCtrl->setTorque(outValue1);

    //has to be the same value as the PID Controller sample time
    os::ThisTask::sleep(std::chrono::milliseconds(5));

    Eigen::Vector3f gravity = g_Mpu->getGravity();
    Trace(ZONE_INFO, "Gravity: x:%6d, y:%6d, z:%6d\n",
          static_cast<int32_t>(gravity.x() * 1000),
          static_cast<int32_t>(gravity.y() * 1000),
          static_cast<int32_t>(gravity.z() * 1000));
}

const os::TaskEndless app::balanceTest(
                                       "PMD_Demo",
                                       4096,
                                       os::Task::Priority::MEDIUM,
                                       [&](const bool&)
    {
                                       setup();
                                       while (1) {
                                           loop();
                                       }
    });
