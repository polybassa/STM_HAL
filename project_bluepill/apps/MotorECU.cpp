// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#include <thread>
#include "Gpio.h"
#include "trace.h"
#include "Can.h"
#include "MotorECU.h"
#include <cstring>
#include "CANFrames.h"

static const int __attribute__((unused)) g_DebugZones = ZONE_ERROR | ZONE_WARNING | ZONE_VERBOSE | ZONE_INFO;

uint32_t speed = 0;
uint32_t fuelLevel = 0;
uint32_t oilLevel = 0;
uint32_t engineTemperature = 0;
uint32_t engineRpm = 0;
uint32_t malFunction = 0;
uint32_t tempomat = 0;
uint32_t ignition = 0;
uint32_t start = 0;
bool challangeSolved = false;

class Delayer
{
    uint32_t mLastExecution = 0;
    uint32_t mDelay = 0;
    std::function<void(void)> mFunc;

public:
    Delayer(std::chrono::milliseconds delay, std::function<void(void)> func) :
        mLastExecution(os::Task::getTickCount()), mDelay(delay.count()), mFunc(func) {}

    bool checkAndExecute(void)
    {
        if (os::Task::getTickCount() > mLastExecution + mDelay) {
            mLastExecution = os::Task::getTickCount();
            if (mFunc) {
                mFunc();
            }
            return true;
        }
        return false;
    }
};

const os::TaskEndless MotorECU("MotorECU",
                               2048, os::Task::Priority::MEDIUM, [](const bool&){
                               constexpr const hal::Can& can = hal::Factory<hal::Can>::get<hal::Can::MAINCAN>();
                               Trace(ZONE_INFO, "Hello MITM Challange MotorECU\r\n");
                               while (true) {
                                   if (!challangeSolved) {
                                       app::msg msg;

                                       os::ThisTask::sleep(std::chrono::milliseconds(300));

                                       msg.setType(app::msg::type::speed);
                                       msg.setData(speed);
                                       can.send(msg);
                                       os::ThisTask::sleep(std::chrono::milliseconds(300));

                                       msg.setType(app::msg::type::fuelLevel);
                                       msg.setData(fuelLevel);
                                       can.send(msg);
                                       os::ThisTask::sleep(std::chrono::milliseconds(300));

                                       msg.setType(app::msg::type::oilLevel);
                                       msg.setData(oilLevel);
                                       can.send(msg);
                                       os::ThisTask::sleep(std::chrono::milliseconds(300));

                                       msg.setType(app::msg::type::engineTemperature);
                                       msg.setData(engineTemperature);
                                       can.send(msg);
                                       os::ThisTask::sleep(std::chrono::milliseconds(300));

                                       msg.setType(app::msg::type::engineRPM);
                                       msg.setData(engineRpm);
                                       can.send(msg);
                                       os::ThisTask::sleep(std::chrono::milliseconds(300));

                                       msg.setType(app::msg::type::malfunction);
                                       msg.setData(malFunction);
                                       can.send(msg);
                                   }
                               }
    });

const os::TaskEndless MotorECUSimu("MotorECUSimu",
                                   2048, os::Task::Priority::MEDIUM, [](const bool&){
                                   constexpr const hal::Gpio& out = hal::Factory<hal::Gpio>::get<hal::Gpio::LED>();
                                   out = true;
                                   Trace(ZONE_INFO, "Hello MITM Challange MotorECU Simulation\r\n");

                                   enum class state {
                                       ignitionOff,
                                       ignitionOn,
                                       motorOn,
                                       challangeSolved
                                   };

                                   state state = state::ignitionOff;

                                   Delayer oilAnimation(std::chrono::milliseconds(300),
                                                        [] {
                                                        oilLevel = ++oilLevel % 15;
        });
                                   Delayer fuelAnimation(std::chrono::milliseconds(300),
                                                         [] {
                                                         fuelLevel = ++fuelLevel % 255;
        });
                                   Delayer speedAnimation(std::chrono::milliseconds(30),
                                                          [] {
                                                          speed = ++speed % 130;
        });
                                   Delayer rpmAnimation(std::chrono::milliseconds(30),
                                                        [] {
                                                        engineRpm = ++engineRpm % 2300;
        });

                                   while (true) {
                                       os::ThisTask::sleep(std::chrono::milliseconds(30));

                                       switch (state) {
                                       case state::ignitionOff:
                                           // Trace(ZONE_INFO, "In ignition off state\r\n");
                                           speed = 0;
                                           oilLevel = 0;
                                           fuelLevel = 0;
                                           engineTemperature = 0;
                                           engineRpm = 0;
                                           malFunction = 0;
                                           start = 0;

                                           ignition != 1 ? state = state::ignitionOff : state = state::ignitionOn;
                                           break;

                                       case state::ignitionOn:
                                           Trace(ZONE_INFO, "In ignition on state\r\n");
                                           speed = 0;
                                           engineRpm = 0;
                                           engineTemperature = 60;
                                           oilAnimation.checkAndExecute();
                                           fuelAnimation.checkAndExecute();

                                           if (ignition & start) {
                                               state = state::motorOn;
                                           }
                                           if (!ignition) {
                                               state = state::ignitionOff;
                                           }
                                           break;

                                       case state::motorOn:
                                           Trace(ZONE_INFO, "In motor on state\r\n");
                                           speedAnimation.checkAndExecute();
                                           rpmAnimation.checkAndExecute();
                                           if (!start & ignition) {
                                               state = state::ignitionOn;
                                           }
                                           if (!start & !ignition) {
                                               state = state::ignitionOff;
                                           }
                                           if(start & !ignition){
                                        	   state = state::ignitionOff;
                                           }
                                           if (tempomat == 130) {
                                               challangeSolved = true;
                                               out = false;
                                               Trace(ZONE_INFO, "Congratulation you passed the Challange !!!\r\n");
                                           }
                                           break;

                                       case state::challangeSolved:
                                           Trace(ZONE_INFO, "Challange solved.\r\n");
                                       }
                                   }
    });

const os::TaskEndless MotorECURx("MotorECURx",
                                 2048, os::Task::Priority::HIGH, [](const bool&) {
                                 constexpr const hal::Can& can = hal::Factory<hal::Can>::get<hal::Can::MAINCAN>();
                                 Trace(ZONE_INFO, "Hello MITM Challange MotorECU Receive\r\n");

                                 while (true) {
                                     if (!challangeSolved) {
                                         os::ThisTask::sleep(std::chrono::milliseconds(1));

                                         CanRxMsg rxmsg;
                                         std::memset(&rxmsg, 0, sizeof(rxmsg));
                                         auto retReceive = can.receive(rxmsg);

                                         if (retReceive) {
                                             app::msg rxm;
                                             memcpy(&rxm, &rxmsg, sizeof(rxm));

                                             switch (rxm.getType()) {
                                             case app::msg::type::tempomat:
                                                 Trace(ZONE_INFO, "Set tempomat to %d\r\n", rxm.getData());
                                                 tempomat = rxm.getData();
                                                 break;

                                             case app::msg::type::ignition:
                                                 Trace(ZONE_INFO, "Set ignition to %d\r\n", rxm.getData());
                                                 ignition = rxm.getData();
                                                 break;

                                             case app::msg::type::start:
                                                 Trace(ZONE_INFO, "Set motor start to %d\r\n", rxm.getData());
                                                 start = rxm.getData();
                                                 break;
                                             }
                                         }
                                     }
                                 }
    });
