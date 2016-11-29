/* Copyright (C) 2016 Nils Weiss */

#include "Test_BALANCE.h"
#include "Mpu.h"
#include "RealTimeDebugInterface.h"

extern app::Mpu* g_mpu;
extern dev::RealTimeDebugInterface* g_RTTerminal;

const os::TaskEndless app::balanceTest("PMD_Demo", 4096, os::Task::Priority::MEDIUM, [] (const bool&){
                                           while (1) {
                                               Eigen::Vector3f accel = g_mpu->getAcceleration();

                                               os::ThisTask::sleep(std::chrono::milliseconds(5));
                                           }
                                       });
