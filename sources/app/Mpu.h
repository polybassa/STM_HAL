/* Copyright (C) 2015  Nils Weiss, Daniel Tatzel, Markus Wildgruber
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
