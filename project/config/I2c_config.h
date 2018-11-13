// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

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
          I2C3_BASE,
          I2C_InitTypeDef { 0x00310309, I2C_AnalogFilter_Enable, 0x00, I2C_Mode_I2C, I2C_OAR1_OA1,
                            I2C_Ack_Enable,
                            I2C_AcknowledgedAddress_7bit })
  }};

static constexpr const std::array<const uint32_t, I2c::__ENUM__SIZE> Clocks =
{ {
      RCC_APB1Periph_I2C3
  }};

#endif /* SOURCES_PMD_I2C_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_I2C_CONFIG_DESCRIPTION_H_ */
