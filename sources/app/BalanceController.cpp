/* Copyright (C) 2016 Nils Weiss
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

#include "BalanceController.h"
#include "trace.h"
#include <cmath>
#include <Eigen/Dense>
#include "Gpio.h"

using app::BalanceController;
using app::MotorController;
using app::Mpu;
using dev::PIDController;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR |
                                                        ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

BalanceController::BalanceController(const Mpu& gyro, MotorController& motor) :
    os::DeepSleepModule(), mBalanceControllerTask("3BalanceControl",
                                                  BalanceController::STACKSIZE, os::Task::Priority::HIGH,
                                                  [this](const bool& join)
                                                  {
                                                      balanceControllerTaskFunction(join);
                                                  }), mMpu(gyro), mMotor(motor), mSetAngleQueue()
{}

void BalanceController::enterDeepSleep(void)
{
    mBalanceControllerTask.join();
}

void BalanceController::exitDeepSleep(void)
{
    mBalanceControllerTask.start();
}

void BalanceController::balanceControllerTaskFunction(const bool& join)
{
    os::ThisTask::sleep(std::chrono::seconds(5));

    Trace(ZONE_INFO, "Start balance controller\r\n");

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

    //Other Variables:
    float sysTickOld = 0;
    float balancingEnabled = 1;

    //Als erstes Motoren ausschalten.
    torqueLeft = 0;
    mMotor.setTorque(torqueLeft);

    //setup PID1 Controller. It needs the sample time of our loop function
    pid1.setSampleTime(mControllerInterval);
    pid1.setOutputLimits(-0.8, 0.8);
    pid1.setMode(dev::PIDController::ControlMode::AUTOMATIC);
    pid1.setControllerDirection(dev::PIDController::ControlDirection::DIRECT);

    //setup PID2 Controller. It needs the sample time of our loop function
    pid2.setSampleTime(mControllerInterval);
    pid2.setOutputLimits(-0.8, 0.8);
    pid2.setMode(dev::PIDController::ControlMode::AUTOMATIC);
    pid2.setControllerDirection(dev::PIDController::ControlDirection::DIRECT);

    do {
    	float angleInputValue = 0.0;

    	mSetAngleQueue.receive(angleInputValue, 0);

        //Print current MPU6050 GravityVector to RTT
        Eigen::Vector3f gravity = mMpu.getGravity();

        //Calculate Vehicle Angle in Deg. *57,296 converts from Rad to Deg. -90 maps Range from -90 > +90deg.
        VehicleAngle = (std::acos(gravity.y()) * 57296 / 1000 - 90);

        //Read RPS from left VESC Inverter
        newMotorSpeedL = (mMotor.getCurrentRPS());

        //update Motorspeed filtered
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

        //Torque for Motors is the Sum of both controller outputs.
        torqueTotal = outValue1 + outValue2;

        //Read torque bias from Poti1 and calculate/map torque bias afterwards. Then send to RTT!
        float valuePoti1 = 0.0;

        torqueBias = valuePoti1;
        torqueBias = (torqueBias - 2048) * 0.3 / 2048;

        //Add torque bias (important for cornering) to both left and right resulting torque values
        //Positive Torque Bias will result in clockwise rotation of the vehicle
        //Range of Torque Bias needs to be determined.
        torqueTotal = outValue1;
        torqueLeft = torqueTotal + torqueBias;
        torqueRight = torqueTotal - torqueBias;

        //update torque. Negative Torque for left inverter.
        mMotor.setTorque(torqueLeft * balancingEnabled);

        //Receive new PID Params from Tracing
        //receiveCommand();

        //GPIO Timing Ausgabe

        //Print if Powertrain is enabled to RTT
        TraceLight("Enabled;%6d;",
                   static_cast<int32_t>(balancingEnabled));
        //Print current sysTick to RTT
        TraceLight("Systick;%8d;",
                   static_cast<int32_t>(os::Task::getTickCount()));
        TraceLight("DeltaSystick;%8d;",
                   static_cast<int32_t>(os::Task::getTickCount() - sysTickOld));

        TraceLight("Gravityx;%6d;",
                   static_cast<int32_t>(gravity.x() * 1000));

        TraceLight("Winkel;%6d;",
                   static_cast<int32_t>(VehicleAngle * 1000));

        TraceLight("LeftRPS;%6d;", static_cast<int32_t>(motorSpeedL));

        TraceLight("Poti1;%d;", static_cast<uint32_t>(valuePoti1));

        TraceLight("torqueBias;%d;",
                   static_cast<int32_t>(torqueBias * 1000));

        TraceLight("PID1;%6d;",
                   static_cast<int32_t>(outValue1 * 1000));
        TraceLight("PID2;%6d;",
                   static_cast<int32_t>(outValue2 * 1000));
        TraceLight("TotalTorque;%6d;",
                   static_cast<int32_t>(torqueTotal * 1000));

        //Output individual Torques to RTT
        TraceLight("MLinks;%6d;",
                   static_cast<int32_t>(torqueLeft * 1000));
        TraceLight("MRechts;%6d;\n",
                   static_cast<int32_t>(torqueRight * 1000));

        //has to be the same valu e as the PID Controller sample time
        sysTickOld = os::Task::getTickCount();

        os::ThisTask::sleep(mControllerInterval);
    } while (!join);
}

void BalanceController::setTargetAngleInDegree(const float angle)
{
    static constexpr float limit = 20.0;

    if (std::abs(angle) < limit) {
        static constexpr float conversionFactor = 360.0 / (2 * M_PI);
        mSetAngleQueue.overwrite(angle / conversionFactor);
    }
}
