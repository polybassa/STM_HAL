/* Copyright (C) 2016  Nils Weiss
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

#include "VescMotorController.h"
#include "trace.h"
#include <cmath>
#include "RealTimeDebugInterface.h"
extern "C" {
#include "vesc_bldc_interface_uart.h"
#include "vesc_bldc_interface.h"
}
#include <functional>
#include "LockGuard.h"
#include "Mutex.h"

using app::VescMotorController;

extern dev::RealTimeDebugInterface* g_RTTerminal;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

constexpr std::chrono::milliseconds VescMotorController::controllerInterval;

static VescMotorController* txMotorControllerPointer = nullptr;
static VescMotorController* rxMotorControllerPointer = nullptr;

static os::Mutex bldcLibraryMutex;

void app::send_packet_callback(unsigned char* data, unsigned int len)
{
    if (txMotorControllerPointer) {
        txMotorControllerPointer->send_packet(data, len);
    }
}

void app::bldc_val_received(void* valPtr)
{
    if (valPtr) {
        mc_values* val = reinterpret_cast<mc_values*>(valPtr);

        //UNCOMMENT for TRACING
        //	g_RTTerminal->printf("Input voltage: %5d mV\r\n",       static_cast<size_t>(val->v_in * 1000));
        //	g_RTTerminal->printf("Temp:          %5d mdegC\r\n",static_cast<size_t>(val->temp_pcb* 1000));
        //	g_RTTerminal->printf("Current motor: %5d mA\r\n",       static_cast<size_t>(val->current_motor* 1000));
        //	g_RTTerminal->printf("Current in:    %5d mA\r\n",       static_cast<size_t>(val->current_in* 1000));
        //	g_RTTerminal->printf("RPM:           %5d RPM\r\n",      static_cast<size_t>(val->rpm* 1000));
        //	g_RTTerminal->printf("Duty cycle:    %5d %%\r\n",       static_cast<size_t>(val->duty_now * 100.0));
        //	g_RTTerminal->printf("Ah Drawn:      %5d mAh\r\n",      static_cast<size_t>(val->amp_hours* 1000));
        //	g_RTTerminal->printf("Ah Regen:      %5d mAh\r\n",      static_cast<size_t>(val->amp_hours_charged* 1000));
        //	g_RTTerminal->printf("Wh Drawn:      %5d mWh\r\n",      static_cast<size_t>(val->watt_hours* 1000));
        //	g_RTTerminal->printf("Wh Regen:      %5d mWh\r\n",      static_cast<size_t>(val->watt_hours_charged* 1000));
        //	g_RTTerminal->printf("Tacho:         %i counts\r\n", val->tachometer);
        //	g_RTTerminal->printf("Tacho ABS:     %i counts\r\n", val->tachometer_abs);
        //	g_RTTerminal->printf("Fault Code:    %s\r\n", bldc_interface_fault_to_string(val->fault_code));

        if (rxMotorControllerPointer) {
            rxMotorControllerPointer->mCurrentRPS = val->rpm / 60;
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
    const size_t receiveValuesPeriode = 50;

    ::bldc_interface_uart_init(send_packet_callback);
    ::bldc_interface_set_rx_value_func(reinterpret_cast<void (*)(mc_values*)>(bldc_val_received));

    if (mInterface.mDescription == hal::Usart::Description::VESC_IF) {
        rxMotorControllerPointer = this;

        mInterface.enableNonBlockingReceive(::bldc_interface_uart_process_byte);
    }

    size_t executionCounter = 0;
    do {
        float newSetTorque;
        if (mSetTorqueQueue.receive(newSetTorque, 0)) {
            constexpr const float CPHI = 0.065;

            os::LockGuard<os::Mutex> lock(bldcLibraryMutex);
            txMotorControllerPointer = this;

            ::bldc_interface_set_current(newSetTorque / CPHI);
        }

        // trigger get values every "receiveValuesPeriode" cycle of this loop
        if ((executionCounter == 0) && (mInterface.mDescription == hal::Usart::Description::VESC_IF)) {
            os::LockGuard<os::Mutex> lock(bldcLibraryMutex);
            txMotorControllerPointer = this;
            ::bldc_interface_get_values();
        }
        executionCounter = (executionCounter + 1) % receiveValuesPeriode;

        bldc_interface_uart_run_timer();
        os::ThisTask::sleep(std::chrono::milliseconds(1));
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
