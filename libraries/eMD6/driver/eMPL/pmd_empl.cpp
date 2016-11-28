/* Copyright (C) 2015  Nils Weiss
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

#include "pmd_empl.h"
#include "I2c.h"
#include "os_Task.h"

int empl_i2c_read(unsigned char  Address,
                  unsigned char  RegisterAddr,
                  unsigned short RegisterLen,
                  unsigned char* RegisterValue)
{
    constexpr auto& interface = hal::Factory<hal::I2c>::get<hal::I2c::GYRO_I2C>();
    const auto retValue = interface.read((Address & 0x7f) << 1, RegisterAddr, RegisterValue, RegisterLen);
    return retValue == RegisterLen ? 0 : -1;
}
int empl_i2c_write(unsigned char        Address,
                   unsigned char        RegisterAddr,
                   unsigned short       RegisterLen,
                   const unsigned char* RegisterValue)
{
    constexpr auto& interface = hal::Factory<hal::I2c>::get<hal::I2c::GYRO_I2C>();
    const auto retValue = interface.write((Address & 0x7f) << 1, RegisterAddr, RegisterValue, RegisterLen);
    return retValue == RegisterLen ? 0 : -1;
}

int empl_get_ms(unsigned long* count)
{
    *count = os::Task::getTickCount();
    return 0;
}
int empl_delay_ms(unsigned long count)
{
    os::ThisTask::sleep(std::chrono::milliseconds(count));
    return 0;
}
