/* Copyright (C) 2016 Nils Weiss */

#include "Test_BALANCE.h"
#include "Mpu.h"
#include "RealTimeDebugInterface.h"
#include "Gpio.h"
#include "Adc.h"
#include "AdcChannel.h"
#include "VescMotorController.h"
#include "PIDController.h"
#include <cmath>

extern app::Mpu* g_mpu;
extern dev::RealTimeDebugInterface* g_RTTerminal;
extern app::VescMotorController* g_motorCtrlL;
extern app::VescMotorController* g_motorCtrlR;

// Get any GPIO you want. You can access them by their name (DESCRIPTION)
constexpr const hal::Gpio& led10 = hal::Factory<hal::Gpio>::get<
                                                                hal::Gpio::LED_10>();
constexpr const hal::Gpio& led3 =
    hal::Factory<hal::Gpio>::get<hal::Gpio::LED_3>();

constexpr const hal::Adc::Channel& poti1 = hal::Factory<hal::Adc::Channel
                                                        > ::get<hal::Adc::Channel::NTC_BATTERY>();
constexpr const hal::Adc::Channel& poti2 = hal::Factory<hal::Adc::Channel
                                                        > ::get<hal::Adc::Channel::NTC_FET>();
constexpr const hal::Adc::Channel& poti3 = hal::Factory<hal::Adc::Channel
                                                        > ::get<hal::Adc::Channel::NTC_MOTOR>();

// Lets prepare a PID Controller for later
float setValue1, currentValue1, outValue1;
const float KP = 0.22;
const float KI = 0;
const float KD = 0.0002;
dev::PIDController pid1(currentValue1, outValue1, setValue1, KP, KI, KD,
                        dev::PIDController::ControlDirection::DIRECT);

//Variables to calculate Vehicle Angle
float VehicleAngle = 0;

//Variables to for torque vectoring
float torqueTotal = 0;
float torqueBias = 0;
float torqueLeft = 0;
float torqueRight = 0;

void setup(void)
{
    g_RTTerminal->printf("Hi Jakob. Here is some setup code\n");
    g_RTTerminal->printf("Turn the motor off, first\n");
    torqueLeft = 0;
    torqueRight = 0;
    g_motorCtrlL->setTorque(torqueLeft);
    g_motorCtrlR->setTorque(torqueRight);

    g_RTTerminal->printf("Wait some ms\n");
    os::ThisTask::sleep(std::chrono::milliseconds(5));

    g_RTTerminal->printf("Now we need more torque\n");
    torqueLeft = 0.2;
    torqueRight = 0.2;
    g_motorCtrlL->setTorque(torqueLeft);
    g_motorCtrlR->setTorque(torqueRight);

    float voltagePoti1 = poti1.getVoltage();
    uint32_t valuePoti1 = poti1.getValue();

    g_RTTerminal->printf(
                         "Poti1 measures %d mV. This is an absolute value of %d\n",
                         static_cast<uint32_t>(voltagePoti1 * 1000), valuePoti1);

    //setup PID Controller. It needs the sample time of our loop function
    pid1.setSampleTime(std::chrono::milliseconds(5));
    pid1.setOutputLimits(-1.0, 1.0);
    pid1.setMode(dev::PIDController::ControlMode::AUTOMATIC);
    pid1.setControllerDirection(dev::PIDController::ControlDirection::DIRECT);
}

// This function will run forever
void loop(void)
{
    Eigen::Vector3f gravity = g_mpu->getGravity();
    g_RTTerminal->printf("Gravity: x:%6d, y:%6d, z:%6d ",
                         static_cast<int32_t>(gravity.x() * 1000),
                         static_cast<int32_t>(gravity.y() * 1000),
                         static_cast<int32_t>(gravity.z() * 1000));

    VehicleAngle = (std::acos(gravity.x()) * 57296 / 1000 - 90);
    g_RTTerminal->printf("Winkel: %6d mDeg ",
                         static_cast<int32_t>(VehicleAngle * 1000));

    // update the current Value of the PID Controller and compute!
    currentValue1 = VehicleAngle;
    setValue1 = 0;
    pid1.compute();

    g_RTTerminal->printf("Total Torque: %6d mNm ",
                         static_cast<int32_t>(outValue1 * 1000));

    //Read torque bias from Poti1 and calculate/map torque bias afterwards.
    float valuePoti1 = poti1.getValue();
    g_RTTerminal->printf(
                         "Poti1 raw: %d ",
                         static_cast<uint32_t>(valuePoti1));

    torqueBias = valuePoti1;
    torqueBias = (torqueBias - 2048) * 0.3 / 2048;

    g_RTTerminal->printf(
                         "torqueBias: %d mNm",
                         static_cast<int32_t>(torqueBias * 1000));

    //Add torque bias (important for cornering) to both left and right resulting torque values
    //Positive Torque Bias will result in clockwise rotation of the vehicle
    //Range of Torque Bias needs to be determined.
    torqueTotal = outValue1;
    torqueLeft = torqueTotal + torqueBias;
    torqueRight = torqueTotal - torqueBias;

    //update torque. Negative Torque for left inverter.
    g_motorCtrlL->setTorque(torqueLeft);
    g_motorCtrlR->setTorque(torqueRight);

    //Debug output
    g_RTTerminal->printf("MLinks:%6dmNm ",
                         static_cast<int32_t>(torqueLeft * 1000));
    g_RTTerminal->printf("MRechts:%6dmNm\n",
                         static_cast<int32_t>(torqueRight * 1000));

    //has to be the same value as the PID Controller sample time
    os::ThisTask::sleep(std::chrono::milliseconds(20));
}

const os::TaskEndless app::balanceTest("PMD_Demo", 4096,
                                       os::Task::Priority::MEDIUM, [&](const bool&)
                                       {
                                           setup();
                                           while (1) {
                                               loop();
                                           }
                                       });
