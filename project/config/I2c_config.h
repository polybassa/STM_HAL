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

#ifndef SOURCES_PMD_I2C_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_I2C_CONFIG_DESCRIPTION_H_

enum Description {
    GYRO_I2C,
    __ENUM__SIZE
};
#else
#ifndef SOURCES_PMD_I2C_CONFIG_CONTAINER_H_
#define SOURCES_PMD_I2C_CONFIG_CONTAINER_H_

/* ATTENTION: Don't forget do add all necessary
 * clock domains to the Factory<I2c>::Clocks array */

static constexpr const std::array<const I2c, I2c::__ENUM__SIZE> Container =
{ {
      I2c(I2c::GYRO_I2C,
          I2C1_BASE,
          I2C_InitTypeDef { 0x00310309, I2C_AnalogFilter_Enable, 0x00, I2C_Mode_I2C, I2C_OAR1_OA1,
                            I2C_Ack_Enable,
                            I2C_AcknowledgedAddress_7bit })
  } };

static constexpr const std::array<const uint32_t, I2c::__ENUM__SIZE> Clocks =
{ {
      RCC_APB1Periph_I2C1
  } };

#endif /* SOURCES_PMD_I2C_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_I2C_CONFIG_DESCRIPTION_H_ */
