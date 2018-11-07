// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "VescMotorController.h"
#include "trace.h"
#include <cmath>
extern "C" {
#include "vesc_bldc_interface_uart.h"
#include "vesc_bldc_interface.h"
}
#include <functional>

using app::VescMotorController;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

constexpr std::chrono::milliseconds VescMotorController::controllerInterval;

static VescMotorController* internalMotorControllerPointer = nullptr;

void app::send_packet_callback(unsigned char* data, unsigned int len)
{
    if (internalMotorControllerPointer) {
        internalMotorControllerPointer->send_packet(data, len);
    }
}

void app::bldc_val_received(void* valPtr)
{
    if (valPtr) {
        mc_values* val = reinterpret_cast<mc_values*>(valPtr);

        //UNCOMMENT for TRACING
        //	Trace(ZONE_INFO, "Input voltage: %5d mV\r\n",       static_cast<size_t>(val->v_in * 1000));
        //	Trace(ZONE_INFO, "Temp:          %5d mdegC\r\n",static_cast<size_t>(val->temp_pcb* 1000));
        //	Trace(ZONE_INFO, "Current motor: %5d mA\r\n",       static_cast<size_t>(val->current_motor* 1000));
        //	Trace(ZONE_INFO, "Current in:    %5d mA\r\n",       static_cast<size_t>(val->current_in* 1000));
        //	Trace(ZONE_INFO, "RPM:           %5d RPM\r\n",      static_cast<size_t>(val->rpm* 1000));
        //	Trace(ZONE_INFO, "Duty cycle:    %5d %%\r\n",       static_cast<size_t>(val->duty_now * 100.0));
        //	Trace(ZONE_INFO, "Ah Drawn:      %5d mAh\r\n",      static_cast<size_t>(val->amp_hours* 1000));
        //	Trace(ZONE_INFO, "Ah Regen:      %5d mAh\r\n",      static_cast<size_t>(val->amp_hours_charged* 1000));
        //	Trace(ZONE_INFO, "Wh Drawn:      %5d mWh\r\n",      static_cast<size_t>(val->watt_hours* 1000));
        //	Trace(ZONE_INFO, "Wh Regen:      %5d mWh\r\n",      static_cast<size_t>(val->watt_hours_charged* 1000));
        //	Trace(ZONE_INFO, "Tacho:         %i counts\r\n", val->tachometer);
        //	Trace(ZONE_INFO, "Tacho ABS:     %i counts\r\n", val->tachometer_abs);
        //	Trace(ZONE_INFO, "Fault Code:    %s\r\n", bldc_interface_fault_to_string(val->fault_code));

        if (internalMotorControllerPointer) {
            internalMotorControllerPointer->mCurrentRPS = val->rpm / 60;
        }
    }
}

VescMotorController::VescMotorController(const hal::Usart& interface) :
    os::DeepSleepModule(),
    mMotorControllerTask("VescMotorControl",
                         VescMotorController::STACKSIZE,
                         os::Task::Priority::LOW,
                         [this](const bool& join)
{
    motorControllerTaskFunction(join);
}),
    mSetTorqueQueue(),
    mInterface(interface)
{
    setTorque(0.00001);
    internalMotorControllerPointer = this;
    ::bldc_interface_uart_init(send_packet_callback);
    ::bldc_interface_set_rx_value_func(reinterpret_cast<void (*)(mc_values*)>(bldc_val_received));
}

void VescMotorController::enterDeepSleep(void)
{
    mMotorControllerTask.join();
}

void VescMotorController::exitDeepSleep(void)
{
    mMotorControllerTask.start();
}

void VescMotorController::send_packet(unsigned char* data, unsigned int len)
{
    mInterface.send(data, len);
}

void VescMotorController::motorControllerTaskFunction(const bool& join)
{
    // if update of RPS Value is to slow, use a smaller value here!
    const size_t receiveValuesPeriode = 1000;

    size_t executionCounter = 0;
    do {
        float newSetTorque;
        if (mSetTorqueQueue.receive(newSetTorque, 0)) {
            constexpr const float CPHI = 0.065;
            ::bldc_interface_set_current(newSetTorque / CPHI);
        }

        // process received data
        const unsigned int arrayLength = 128;
        unsigned char dataArray[arrayLength];

        const size_t receivedBytes = mInterface.receiveAvailableData(dataArray, arrayLength);

        for (size_t i = 0; i < receivedBytes; i++) {
            ::bldc_interface_uart_process_byte(dataArray[i]);
        }

        // trigger get values every "receiveValuesPeriode" cycle of this loop
        if (executionCounter == 0) {
            ::bldc_interface_get_values();
        }
        executionCounter = (executionCounter + 1) % receiveValuesPeriode;
    } while (!join);
}

void VescMotorController::setTorque(const float setValue)
{
    mSetTorqueQueue.overwrite(setValue);
}

float VescMotorController::getCurrentRPS(void) const
{
    return mCurrentRPS;
}
