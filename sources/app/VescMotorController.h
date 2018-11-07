// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

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
    private os::DeepSleepModule, public interface ::MotorController
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
    VescMotorController(const hal::Usart& interface);

    VescMotorController(const VescMotorController&) = delete;
    VescMotorController(VescMotorController&&) = delete;
    VescMotorController& operator=(const VescMotorController&) = delete;
    VescMotorController& operator=(VescMotorController&&) = delete;

    virtual void setTorque(const float) override;
    virtual float getCurrentRPS(void) const override;

#ifdef UNITTEST
    void triggerTaskExecution(void) { this->motorControllerTaskFunction(true); }
#endif
    friend void send_packet_callback(unsigned char*, unsigned int);
    friend void bldc_val_received(void* valPtr);
};
}
