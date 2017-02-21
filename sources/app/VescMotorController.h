/* Copyright (C) 2016  Nils Weiss
 # *
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

#ifndef SOURCES_PMD_VESCMOTORCONTROLLER_H_
#define SOURCES_PMD_VESCMOTORCONTROLLER_H_

#include "interface_MotorController.h"
#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "os_Queue.h"
#include "Usart.h"
#include <limits>

namespace app
{
void send_packet_callback(unsigned char*, unsigned int);
void bldc_val_received(void* valPtr);

class VescMotorController final :
    private os::DeepSleepModule, public interface::MotorController
{
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr uint32_t STACKSIZE = 2048;

    os::TaskInterruptable mMotorControllerTask;
    os::Queue<float, 1> mSetTorqueQueue;

    const hal::Usart& mInterface;
    float mCurrentRPS;

    static constexpr std::chrono::milliseconds controllerInterval = std::chrono::milliseconds(2);

    void motorControllerTaskFunction(const bool&);
    void send_packet(unsigned char* data, unsigned int len);

public:
    VescMotorController(const hal::Usart & interface);

    VescMotorController(const VescMotorController &) = delete;
    VescMotorController(VescMotorController &&) = delete;
    VescMotorController& operator=(const VescMotorController&) = delete;
    VescMotorController& operator=(VescMotorController &&) = delete;

    virtual void setTorque(const float) override;
    virtual float getCurrentRPS(void) const override;

#ifdef UNITTEST
    void triggerTaskExecution(void) { this->motorControllerTaskFunction(true); }
#endif
    friend void send_packet_callback(unsigned char*, unsigned int);
    friend void bldc_val_received(void* valPtr);
};
}

#endif /* SOURCES_PMD_VESCMOTORCONTROLLER_H_ */
