/* Copyright (C) 2016 Nils Weiss */

#include "Test_BALANCE.h"
#include "Mpu.h"
#include "RealTimeDebugInterface.h"
#include "Gpio.h"
#include "Adc.h"
#include "AdcChannel.h"
#include "VescMotorController.h"
#include "PIDController.h"
#include "SEGGER_RTT.h"
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

// use global definition, to feed same time to the PID controller and the sleep function
std::chrono::milliseconds WAITTIME = std::chrono::milliseconds(20);

void receiveCommand(void)
{
    //PARSE FOR INPUT:P=1234 D=1234<ENTER>

    static constexpr const size_t BUFFERSIZE = 100;

    static char buffer[BUFFERSIZE];
    static size_t index = 0;

    static bool values_Stored = false;
    static int p_Stored = 0;
    static int d_Stored = 0;

    if (SEGGER_RTT_HasKey()) {
        buffer[index] = static_cast<char>(SEGGER_RTT_GetKey());
        index = (index + 1) % BUFFERSIZE;
    }

    char* delimeterPosition = reinterpret_cast<char*>(memchr(buffer, '\n', BUFFERSIZE));

    if (delimeterPosition == nullptr) {
        return;
    }

    const size_t endIndex = reinterpret_cast<size_t>(delimeterPosition) - reinterpret_cast<size_t>(buffer);

    buffer[endIndex] = 0;

    char* parameter_P_Position = strstr(buffer, "P=");
    char* parameter_D_Position = strstr(buffer, "D=");

    char parameter_P[4];
    char parameter_D[4];

    if ((parameter_D_Position == nullptr) || (parameter_P_Position == nullptr)) {
        g_RTTerminal->printf("P= and D= not found\n");
        values_Stored = false;
        p_Stored = 0;
        d_Stored = 0;
        index = 0;
        memset(buffer, 0, BUFFERSIZE);
        return;
    }
    memcpy(parameter_P, parameter_P_Position + 2, 4);
    memcpy(parameter_D, parameter_D_Position + 2, 4);

    for (size_t i = 0; i < 4; i++) {
        if ((parameter_P[i] > '9') || (parameter_P[i] < '0')) {
            g_RTTerminal->printf("Input error Parameter P\n");
            values_Stored = false;
            p_Stored = 0;
            d_Stored = 0;
            index = 0;
            memset(buffer, 0, BUFFERSIZE);
            return;
        }
    }

    for (size_t i = 0; i < 4; i++) {
        if ((parameter_D[i] > '9') || (parameter_D[i] < '0')) {
            g_RTTerminal->printf("Input error Parameter D\n");
            values_Stored = false;
            p_Stored = 0;
            d_Stored = 0;
            index = 0;
            memset(buffer, 0, BUFFERSIZE);
            return;
        }
    }

    int p = atoi(parameter_P);
    int d = atoi(parameter_D);

    if (!values_Stored) {
        values_Stored = true;
        p_Stored = p;
        d_Stored = d;
        index = 0;
        memset(buffer, 0, BUFFERSIZE);
        g_RTTerminal->printf("Stored P=%2d.%2d D=%2d.%2d\n", p / 100, p % 100, d / 100, d % 100);
        return;
    } else {
        if ((p == p_Stored) && (d == d_Stored)) {
            g_RTTerminal->printf("New P %d.%d and D %d.%d will be applied\n", p / 100, p % 100, d / 100, d % 100);

            pid1.setTunings(static_cast<float>(p) / 100, KI, static_cast<float>(d) / 100);
        } else {
            g_RTTerminal->printf("Error: Stored P=%4d D=%4d, Read P=%4d D=%4d\n", p_Stored, d_Stored, p, d);
        }
    }

    values_Stored = false;
    p_Stored = 0;
    d_Stored = 0;
    index = 0;
    memset(buffer, 0, BUFFERSIZE);
}

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
    pid1.setSampleTime(WAITTIME);
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
    g_RTTerminal->printf("Poti1 raw: %d ",
                         static_cast<uint32_t>(valuePoti1));

    torqueBias = valuePoti1;
    torqueBias = (torqueBias - 2048) * 0.3 / 2048;

    g_RTTerminal->printf("torqueBias: %d mNm",
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

    receiveCommand();

    //has to be the same value as the PID Controller sample time
    os::ThisTask::sleep(WAITTIME);
}

const os::TaskEndless app::balanceTest("PMD_Demo", 4096,
                                       os::Task::Priority::MEDIUM, [&](const bool&)
                                       {
                                           setup();
                                           while (1) {
                                               loop();
                                           }
                                       });
