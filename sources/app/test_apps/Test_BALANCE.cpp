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
#include "Light.h"
#include <cmath>

extern app::Mpu* g_mpu;
extern dev::RealTimeDebugInterface* g_RTTerminal;
extern app::VescMotorController* g_motorCtrlL;
extern app::VescMotorController* g_motorCtrlR;

// Get any GPIO you want. You can access them by their name (DESCRIPTION)
constexpr const hal::Gpio& led10 = hal::Factory<hal::Gpio>::get<hal::Gpio::LED_10>();
constexpr const hal::Gpio& led3 = hal::Factory<hal::Gpio>::get<hal::Gpio::LED_3>();
constexpr const hal::Gpio& userbutton = hal::Factory<hal::Gpio>::get<hal::Gpio::USER_BUTTON>();

constexpr const hal::Adc::Channel& poti1 = hal::Factory<hal::Adc::Channel
                                                        > ::get<hal::Adc::Channel::NTC_BATTERY>();
constexpr const hal::Adc::Channel& poti2 = hal::Factory<hal::Adc::Channel
                                                        > ::get<hal::Adc::Channel::NTC_FET>();
constexpr const hal::Adc::Channel& poti3 = hal::Factory<hal::Adc::Channel
                                                        > ::get<hal::Adc::Channel::NTC_MOTOR>();

//PID 1 for angle dependent Control
float setValue1, currentValue1, outValue1;
const float KP1 = 0.26;
const float KI1 = 0;
const float KD1 = 0.0002;
dev::PIDController pid1(currentValue1, outValue1, setValue1, KP1, KI1, KD1,
                        dev::PIDController::ControlDirection::DIRECT);

//PID 2 for speed dependent Control
float setValue2, currentValue2, outValue2;
const float KP2 = 0.0005;
const float KI2 = 0;
const float KD2 = 0;
dev::PIDController pid2(currentValue2, outValue2, setValue2, KP2, KI2, KD2,
                        dev::PIDController::ControlDirection::DIRECT);

//Variables to calculate Vehicle Angle
float VehicleAngle = 0;

//Variables to for torque vectoring
float torqueTotal = 0;
float torqueBias = 0;
float torqueLeft = 0;
float torqueRight = 0;
float motorSpeedL = 0;
float newMotorSpeedL = 0;

float balancingEnabled = 0;

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

            pid1.setTunings(static_cast<float>(p) / 100, KI1, static_cast<float>(d) / 100);
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
    //Als erstes Motoren ausschalten.
    g_RTTerminal->printf("Turn the motor off, first\n");
    torqueLeft = 0;
    torqueRight = 0;
    g_motorCtrlL->setTorque(torqueLeft);
    g_motorCtrlR->setTorque(torqueRight);

    //setup PID1 Controller. It needs the sample time of our loop function
    pid1.setSampleTime(WAITTIME);
    pid1.setOutputLimits(-0.8, 0.8);
    pid1.setMode(dev::PIDController::ControlMode::AUTOMATIC);
    pid1.setControllerDirection(dev::PIDController::ControlDirection::DIRECT);

    //setup PID2 Controller. It needs the sample time of our loop function
    pid2.setSampleTime(WAITTIME);
    pid2.setOutputLimits(-0.8, 0.8);
    pid2.setMode(dev::PIDController::ControlMode::AUTOMATIC);
    pid2.setControllerDirection(dev::PIDController::ControlDirection::DIRECT);
}

// This function will run forever
void loop(void)
{
    //Als erstes checken, ob der Taster am Lenker gedrückt ist und bei Bedarf LED 3 einschalten
    if (userbutton == true) {
        balancingEnabled = 1;
        led3 = true;
    } else {
        balancingEnabled = 0;
        led3 = false;
    }

    g_RTTerminal->printf("Enabled: %6d ",
                         static_cast<int32_t>(balancingEnabled));

    Eigen::Vector3f gravity = g_mpu->getGravity();
    g_RTTerminal->printf("Gravity: x:%6d, y:%6d, z:%6d ",
                         static_cast<int32_t>(gravity.x() * 1000),
                         static_cast<int32_t>(gravity.y() * 1000),
                         static_cast<int32_t>(gravity.z() * 1000));

    VehicleAngle = (std::acos(gravity.x()) * 57296 / 1000 - 90);
    g_RTTerminal->printf("Winkel: %6d mDeg ",
                         static_cast<int32_t>(VehicleAngle * 1000));

    //Read RPS from left VESC Inverter
    newMotorSpeedL = (g_motorCtrlL->getCurrentRPS());
    g_RTTerminal->printf("LeftRPS: %6d ",
                         static_cast<int32_t>(motorSpeedL));

    //update MOtorspeed filtered
    constexpr const float FILTERSIZE = 32;
    motorSpeedL -= motorSpeedL / FILTERSIZE;
    motorSpeedL += newMotorSpeedL / FILTERSIZE;

    // update the current Value of the PID Controllers and compute!
    currentValue1 = VehicleAngle;
    currentValue2 = motorSpeedL;

    setValue1 = 0;
    setValue2 = 0;
    pid1.compute();
    pid2.compute();

    torqueTotal = outValue1 + outValue2;

    g_RTTerminal->printf("PID1: %6d mNm ",
                         static_cast<int32_t>(outValue1 * 1000));
    g_RTTerminal->printf("PID2: %6d mNm ",
                         static_cast<int32_t>(outValue2 * 1000));
    g_RTTerminal->printf("Total Torque: %6d mNm ",
                         static_cast<int32_t>(torqueTotal * 1000));

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
    g_motorCtrlL->setTorque(torqueLeft * balancingEnabled);
    g_motorCtrlR->setTorque(torqueRight * balancingEnabled);

    //Debug output
    g_RTTerminal->printf("MLinks:%6dmNm ",
                         static_cast<int32_t>(torqueLeft * 1000));
    g_RTTerminal->printf("MRechts:%6dmNm\n",
                         static_cast<int32_t>(torqueRight * 1000));

    //Receive new PID Params from Tracing
    receiveCommand();

    //has to be the same valu e as the PID Controller sample time
    os::ThisTask::sleep(WAITTIME);
}

constexpr const dev::Light& backLight = dev::Factory<dev::Light>::get<interface::Light::BACKLIGHT>();
constexpr const dev::Light& headLight = dev::Factory<dev::Light>::get<interface::Light::HEADLIGHT>();

void setup_LED(void)
{
    for (uint8_t i = 0; i < 255; i++) {
        backLight.setColor({0, 0, i});
        headLight.setColor({0, 0, i});

        os::ThisTask::sleep(std::chrono::milliseconds(10));
    }
    backLight.setColor({0, 0, 0});
    headLight.setColor({0, 0, 0});
}

void loop_LED(void)
{
    static size_t state = 0;

    Eigen::Vector3f gravity = g_mpu->getGravity();

    float tempvehicleAngle = gravity.x();
    float tempvehicleRotation = std::abs(gravity.y());

    if (tempvehicleRotation > 0.6) {
        //charging

        constexpr const size_t timeForOneLed_ms = 200 / 10;
        static size_t iteration = 0;

        if (iteration == 0) {
            state = ++state;
            if (state > 21) {state = 21; }

            backLight.displayNumber(state, {0, 30, 0});
            headLight.displayNumber(state, {0, 30, 0});
        }

        iteration++;
        iteration = iteration % timeForOneLed_ms;
    } else {
        state = 0;
        static bool direction = true;

        if ((tempvehicleAngle > 0.1) && (direction == false)) {
            direction = true;
        } else if ((tempvehicleAngle < -0.1) && (direction == true)) {
            direction = false;
        }

        if (!direction) {
            headLight.setColor({static_cast<uint8_t>(30 * tempvehicleAngle + 10),
                                static_cast<uint8_t>(30 * tempvehicleAngle + 10),
                                static_cast<uint8_t>(30 * tempvehicleAngle + 10)});
            backLight.setColor({static_cast<uint8_t>(90 * tempvehicleAngle + 10), 0, 0});
        } else {
            backLight.setColor({static_cast<uint8_t>(30 * tempvehicleAngle + 10),
                                static_cast<uint8_t>(30 * tempvehicleAngle + 10),
                                static_cast<uint8_t>(30 * tempvehicleAngle + 10)});
            backLight.setColor({static_cast<uint8_t>(90 * tempvehicleAngle + 10), 0, 0});
        }
    }
    os::ThisTask::sleep(std::chrono::milliseconds(10));
}

const os::TaskEndless app::balanceTest("PMD_Demo", 4096,
                                       os::Task::Priority::MEDIUM, [&](const bool&)
                                       {
                                           setup();
                                           while (1) {
                                               loop();
                                           }
                                       });

const os::TaskEndless app::ledTest("LED_Demo", 4096,
                                   os::Task::Priority::LOW, [&](const bool&)
                                   {
                                       setup_LED();
                                       while (1) {
                                           loop_LED();
                                       }
                                   });
