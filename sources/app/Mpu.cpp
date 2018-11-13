// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include "Mpu.h"
#include "Exti.h"
#include "LockGuard.h"
#include "trace.h"
#include <cmath>

#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "eMPL_outputs.h"
#include "invensense.h"
#include "invensense_adv.h"

using app::Mpu;

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;
unsigned char* mpl_key = (unsigned char*)"eMPL 5.1"; /* Has to be defined for Library */
os::Semaphore Mpu::UpdateDataSemaphore;
constexpr const std::chrono::milliseconds Mpu::UpdateTemperatureInterval;

void Mpu::MpuInterruptHandler(void)
{
    UpdateDataSemaphore.giveFromISR();
}

static void tap_cb(unsigned char direction, unsigned char count)
{}

static void android_orient_cb(unsigned char orientation)
{}

Mpu::Mpu(const hal::Exti& exti) :
    os::DeepSleepModule(),
    mMpuTask("2mpuTask",
             Mpu::STACKSIZE,
             os::Task::Priority::HIGH,
             [this](const bool& join)
{
    mpuTaskFunction(join);
}), mExti(exti)
{
    mExti.registerInterruptCallback(MpuInterruptHandler);
    mExti.enable();

    int_param_s int_param;

    if (mpu_init(&int_param)) {
        Trace(ZONE_ERROR, "Could not initialize gyro\r\n");
        return;
    }

    if (inv_init_mpl()) {
        Trace(ZONE_ERROR, "Could not initialize MPL\r\n");
        return;
    }

    /* Compute 6-axis quaternions. */
    inv_enable_quaternion();

    /* Update gyro biases when not in motion.
     * WARNING: These algorithms are mutually exclusive.
     */
    //inv_enable_fast_nomot();
    //inv_enable_in_use_auto_calibration();
    //inv_enable_motion_no_motion();
    //inv_set_no_motion_time(100);

    /* Allows use of the MPL APIs in read_from_mpl. */
    inv_enable_eMPL_outputs();

    /* Update gyro biases when temperature changes. */
    inv_enable_gyro_tc();

    if (inv_start_mpl()) {
        Trace(ZONE_ERROR, "Could not start MPL\r\n");
        return;
    }

    /* Get/set hardware configuration. Start gyro. */
    /* Wake up all sensors. */
    mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);

    /* Push both gyro and accel data into the FIFO. */
    mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);

    mpu_set_sample_rate(DEFAULT_MPU_HZ);

    unsigned char accel_fsr;
    unsigned short gyro_rate, gyro_fsr;
    /* Read back configuration in case it was set improperly. */
    mpu_get_sample_rate(&gyro_rate);
    mpu_get_gyro_fsr(&gyro_fsr);
    mpu_get_accel_fsr(&accel_fsr);

    /* Sync driver configuration with MPL. */
    /* Sample rate expected in microseconds. */
    inv_set_gyro_sample_rate(1000000L / gyro_rate);
    inv_set_accel_sample_rate(1000000L / gyro_rate);

    /* Set chip-to-body orientation matrix.
     * Set hardware units to dps/g's/degrees scaling factor.
     *
     *  STM32F3  Matrix:
     *	1, 0, 0,
     *  0, 1, 0,
     *  0, 0, 1
     */
    signed char orientation[9] = {
        1, 0, 0,
        0, 1, 0,
        0, 0, 1
    };

    inv_set_gyro_orientation_and_scale(inv_orientation_matrix_to_scalar(orientation),
                                       (long)gyro_fsr << 15);
    inv_set_accel_orientation_and_scale(inv_orientation_matrix_to_scalar(orientation),
                                        (long)accel_fsr << 15);

    /* To initialize the DMP:
     * 1. Call dmp_load_motion_driver_firmware(). This pushes the DMP image in
     *    inv_mpu_dmp_motion_driver.h into the MPU memory.
     * 2. Push the gyro and accel orientation matrix to the DMP.
     * 3. Register gesture callbacks. Don't worry, these callbacks won't be
     *    executed unless the corresponding feature is enabled.
     * 4. Call dmp_enable_feature(mask) to enable different features.
     * 5. Call dmp_set_fifo_rate(freq) to select a DMP output rate.
     * 6. Call any feature-specific control functions.
     *
     * To enable the DMP, just call mpu_set_dmp_state(1). This function can
     * be called repeatedly to enable and disable the DMP at runtime.
     *
     * The following is a short summary of the features supported in the DMP
     * image provided in inv_mpu_dmp_motion_driver.c:
     * DMP_FEATURE_LP_QUAT: Generate a gyro-only quaternion on the DMP at
     * 200Hz. Integrating the gyro data at higher rates reduces numerical
     * errors (compared to integration on the MCU at a lower sampling rate).
     * DMP_FEATURE_6X_LP_QUAT: Generate a gyro/accel quaternion on the DMP at
     * 200Hz. Cannot be used in combination with DMP_FEATURE_LP_QUAT.
     * DMP_FEATURE_TAP: Detect taps along the X, Y, and Z axes.
     * DMP_FEATURE_ANDROID_ORIENT: Google's screen rotation algorithm. Triggers
     * an event at the four orientations where the screen should rotate.
     * DMP_FEATURE_GYRO_CAL: Calibrates the gyro data after eight seconds of
     * no motion.
     * DMP_FEATURE_SEND_RAW_ACCEL: Add raw accelerometer data to the FIFO.
     * DMP_FEATURE_SEND_RAW_GYRO: Add raw gyro data to the FIFO.
     * DMP_FEATURE_SEND_CAL_GYRO: Add calibrated gyro data to the FIFO. Cannot
     * be used in combination with DMP_FEATURE_SEND_RAW_GYRO.
     */
    dmp_load_motion_driver_firmware();
    dmp_set_orientation(inv_orientation_matrix_to_scalar(orientation));
    dmp_register_tap_cb(tap_cb);
    dmp_register_android_orient_cb(android_orient_cb);
    /*
     * Known Bug -
     * DMP when enabled will sample sensor data at 200Hz and output to FIFO at the rate
     * specified in the dmp_set_fifo_rate API. The DMP will then sent an interrupt once
     * a sample has been put into the FIFO. Therefore if the dmp_set_fifo_rate is at 25Hz
     * there will be a 25Hz interrupt from the MPU device.
     *
     * There is a known issue in which if you do not enable DMP_FEATURE_TAP
     * then the interrupts will be at 200Hz even if fifo rate
     * is set at a different rate. To avoid this issue include the DMP_FEATURE_TAP
     *
     * DMP sensor fusion works only with gyro at +-2000dps and accel +-2G
     */
    unsigned short dmp_features = DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
                                  DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO |
                                  DMP_FEATURE_GYRO_CAL;
    dmp_enable_feature(dmp_features);
    dmp_set_fifo_rate(DEFAULT_MPU_HZ);
}

void Mpu::enterDeepSleep(void)
{
    mMpuTask.join();
}

void Mpu::exitDeepSleep(void)
{
    mMpuTask.start();
}

void Mpu::calibrate(void) const
{
    inv_disable_fast_nomot();
    inv_disable_in_use_auto_calibration();
    auto biases = getBiases();
    setBiases(biases);
}

void Mpu::mpuTaskFunction(const bool& join)
{
    //start code
    mpu_set_dmp_state(1);

    bool update_temperature = false;
    auto lastTemperatureUpdate = os::Task::getTickCount();

    do {
        UpdateDataSemaphore.take();
        {
            os::LockGuard<os::Mutex> lock(mUpdateDataMutex);

            if (lastTemperatureUpdate + UpdateTemperatureInterval.count() < os::Task::getTickCount()) {
                update_temperature = true;
                lastTemperatureUpdate = os::Task::getTickCount();
            }

            bool new_data = false;
            /* This function gets new data from the FIFO when the DMP is in
             * use. The FIFO can contain any combination of gyro, accel,
             * quaternion, and gesture data. The sensors parameter tells the
             * caller which data fields were actually populated with new data.
             * For example, if sensors == (INV_XYZ_GYRO | INV_WXYZ_QUAT), then
             * the FIFO isn't being filled with accel data.
             * The driver parses the gesture data to determine if a gesture
             * event has occurred; on an event, the application will be notified
             * via a callback (assuming that a callback function was properly
             * registered). The more parameter is non-zero if there are
             * leftover packets in the FIFO.
             */
            unsigned long sensor_timestamp = 0;
            long accel[3], quat[4];
            short gyro[3], accel_short[3], sensors = 0;
            unsigned char more = 0;

            dmp_read_fifo(gyro, accel_short, quat, &sensor_timestamp, &sensors, &more);
            if (more) {
                UpdateDataSemaphore.give();
            }
            if (sensors & INV_XYZ_GYRO) {
                /* Push the new data to the MPL. */
                inv_build_gyro(gyro, sensor_timestamp);
                new_data = true;
            }

            if (sensors & INV_XYZ_ACCEL) {
                accel[0] = (long)accel_short[0];
                accel[1] = (long)accel_short[1];
                accel[2] = (long)accel_short[2];
                inv_build_accel(accel, 0, sensor_timestamp);
                new_data = true;
            }
            if (sensors & INV_WXYZ_QUAT) {
                inv_build_quat(quat, 0, sensor_timestamp);
                new_data = true;
            }
            if (update_temperature && new_data) {
                long temperature = 0;
                update_temperature = false;
                /* Temperature only used for gyro temp comp. */
                mpu_get_temperature(&temperature, &sensor_timestamp);
                inv_build_temp(temperature, sensor_timestamp);
            }

            if (new_data) {
                inv_execute_on_data();
            }
        }
    } while (!join);

    mpu_set_dmp_state(0);
    // stop code
}

std::pair<Eigen::Vector3i, Eigen::Vector3i> Mpu::getBiases(void) const
{
    long gyro[3], accel[3];
    mpu_run_self_test(gyro, accel);
    return {
               {
                   gyro[0], gyro[1], gyro[2]
               },
               {
                   accel[0], accel[1], accel[2]
               }
    };
}

void Mpu::setBiases(const std::pair<Eigen::Vector3i, Eigen::Vector3i>& values) const
{
    setBiasesGyro(values.first);
    setBiasesAccel(values.second);
}
void Mpu::setBiasesGyro(const Eigen::Vector3i& gyroBiases) const
{
    long biases[3];
    biases[0] = gyroBiases.x();
    biases[1] = gyroBiases.y();
    biases[2] = gyroBiases.z();
    dmp_set_gyro_bias(biases);
}

void Mpu::setBiasesAccel(const Eigen::Vector3i& accelBiases) const
{
    long biases[3];
    biases[0] = accelBiases.x();
    biases[1] = accelBiases.y();
    biases[2] = accelBiases.z();
    dmp_set_accel_bias(biases);
}

Eigen::Vector3f Mpu::getGravity(void) const
{
    long data[3];

    os::LockGuard<os::Mutex> lock(mUpdateDataMutex);

    bool __attribute__((unused)) updated = inv_get_gravity(data);

    return Eigen::Vector3f(data[0] / TWO_PWO_30, data[1] / TWO_PWO_30, data[2] / TWO_PWO_30);
}

Eigen::Vector4f Mpu::getRotationAndDegrees(void) const
{
    auto q = getQuaternion();

    float w = 2 * asin(q.w() / TWO_PWO_30);
    float x = (q.x() / TWO_PWO_30) / (sin(w / 2));
    float y = (q.y() / TWO_PWO_30) / (sin(w / 2));
    float z = (q.z() / TWO_PWO_30) / (sin(w / 2));
    return Eigen::Vector4f(w, x, y, z);
}

Eigen::Vector3f Mpu::getEuler(void) const
{
    long data[3];
    int8_t accuracy;
    inv_time_t timestamp;

    os::LockGuard<os::Mutex> lock(mUpdateDataMutex);

    bool __attribute__((unused)) updated = inv_get_sensor_type_euler(data, &accuracy, &timestamp);

    float y = data[0] / TWO_PWO_16;
    float x = data[1] / TWO_PWO_16;
    float z = data[2] / TWO_PWO_16;
    return Eigen::Vector3f(x, y, z);
}

Eigen::Vector3f Mpu::getAcceleration(void) const
{
    long data[3];
    int8_t accuracy;
    inv_time_t timestamp;

    os::LockGuard<os::Mutex> lock(mUpdateDataMutex);

    bool __attribute__((unused)) updated = inv_get_sensor_type_accel(data, &accuracy, &timestamp);

    float y = data[0] / TWO_PWO_16;
    float x = data[1] / TWO_PWO_16;
    float z = data[2] / TWO_PWO_16;
    return Eigen::Vector3f(x, y, z);
}

Eigen::Quaternionf Mpu::getQuaternion(void) const
{
    long data[4];
    int8_t accuracy;
    inv_time_t timestamp;

    os::LockGuard<os::Mutex> lock(mUpdateDataMutex);

    bool __attribute__((unused)) updated = inv_get_sensor_type_quat(data, &accuracy, &timestamp);
    return {
               static_cast<float>(data[0]), static_cast<float>(data[1]),
               static_cast<float>(data[2]), static_cast<float>(data[3])
    };
}

float Mpu::getGyro(void) const
{
    long data[1];
    int8_t accuracy;
    inv_time_t timestamp;

    os::LockGuard<os::Mutex> lock(mUpdateDataMutex);

    bool __attribute__((unused)) updated = inv_get_sensor_type_gyro(data, &accuracy, &timestamp);
    return data[0] / TWO_PWO_16;
}
