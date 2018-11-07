// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#pragma once

#include "TaskInterruptable.h"
#include "DeepSleepInterface.h"
#include "Mutex.h"
#include "Semaphore.h"
#include "Exti.h"
#include <Eigen/Dense>

namespace app
{
class Mpu final :
    private os::DeepSleepModule
{
    virtual void enterDeepSleep(void) override;
    virtual void exitDeepSleep(void) override;

    static constexpr uint32_t STACKSIZE = 1024;
    static os::Semaphore UpdateDataSemaphore;

    os::TaskInterruptable mMpuTask;
    mutable os::Mutex mUpdateDataMutex;
    static constexpr const std::chrono::milliseconds UpdateTemperatureInterval = std::chrono::milliseconds(500);
    const hal::Exti& mExti;

    void setBiasesAccel(const Eigen::Vector3i&) const;
    void setBiasesGyro(const Eigen::Vector3i&) const;

    void mpuTaskFunction(const bool&);
    void setBiases(const std::pair<Eigen::Vector3i, Eigen::Vector3i>&) const;

public:
    Mpu(const hal::Exti& exti = hal::Factory<hal::Exti>::get<hal::Exti::IMU_INT>());

    Mpu(const Mpu&) = delete;
    Mpu(Mpu&&) = delete;
    Mpu& operator=(const Mpu&) = delete;
    Mpu& operator=(Mpu&&) = delete;

    static constexpr unsigned short DEFAULT_MPU_HZ = 200;
    static constexpr double TWO_PWO_16 = 65536.f;
    static constexpr double TWO_PWO_30 = 1073741824.f;

    std::pair<Eigen::Vector3i, Eigen::Vector3i> getBiases(void) const;
    Eigen::Vector4f getRotationAndDegrees(void) const;
    Eigen::Quaternionf getQuaternion(void) const;
    Eigen::Vector3f getGravity(void) const;
    Eigen::Vector3f getEuler(void) const;
    Eigen::Vector3f getAcceleration(void) const;
    float getGyro(void) const;
    void calibrate(void) const;

    static void MpuInterruptHandler(void);
};
}
