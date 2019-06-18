/* Copyright (C) 2018  Nils Weiss, Florian Breintner, Markus Wildgruber and Henning Mende
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

#ifndef SOURCES_PMD_I2C_H_
#define SOURCES_PMD_I2C_H_

#include "stm32f4xx_i2c.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx.h"
#include <cstdint>
#include <array>
#include "hal_Factory.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "Mutex.h"

///////////////////////////////////////////////////////////////////////////////
#define IS_I2C_ALL_PERIPH_BASE(PERIPH) (((PERIPH) == I2C1_BASE) || \
                                        ((PERIPH) == I2C2_BASE) || \
                                        ((PERIPH) == I2C3_BASE))
///////////////////////////////////////////////////////////////////////////////
/**
 * Note: Currently only master mode is implemented.
 * Missing:
 *  - 10 bit addressing
 *  - Slave mode
 *  - Multimaster
 */
namespace hal
{
struct I2c {
#include "I2c_config.h"

    I2c() = delete;
    I2c(const I2c&) = delete;
    I2c(I2c&&) = default;
    I2c& operator=(const I2c&) = delete;
    I2c& operator=(I2c&&) = delete;

    size_t write(const uint16_t deviceAddr, const uint8_t regAddr, uint8_t const* const data,
                 const size_t length) const;
    size_t write(const uint16_t deviceAddr, uint8_t const* const data, const size_t length) const;
    size_t read(const uint16_t deviceAddr, const uint8_t regAddr, uint8_t* const data, const size_t length) const;

private:
    constexpr I2c(const enum Description& desc,
                  const uint32_t&         peripherie,
                  const I2C_InitTypeDef&  conf) :
        mDescription(desc), mPeripherie(peripherie), mConfiguration(conf) {}

    const enum Description mDescription;
    const uint32_t mPeripherie;
    const I2C_InitTypeDef mConfiguration;

    static constexpr const uint32_t TIMEOUT = 0x10000; // Timeout for I2c write/read routines

    void initialize(void) const;
    bool timeoutDuringWaitUntilFlagIsEqualState(const uint32_t flag, const FlagStatus state) const;
    bool timeoutDuringWaitEvent(const uint32_t event) const;
    bool initMasterCommunication(const uint16_t deviceAddr, const size_t length) const;
    bool sendRegisterAddress(const uint16_t deviceAddr, const uint8_t regAddr, const size_t length) const;

    static std::array<os::Mutex, Description::__ENUM__SIZE> MutexArray;

    friend class Factory<I2c>;
};

template<>
class Factory<I2c>
{
#include "I2c_config.h"

    Factory(void)
    {
        for (const auto& clock : Clocks) {
            RCC_APB1PeriphClockCmd(clock, ENABLE);
        }
        for (const auto& I2c : Container) {
            I2c.initialize();
        }
    }

public:
    template<enum I2c::Description index>
    static constexpr const I2c& get(void)
    {
        static_assert(IS_I2C_ALL_PERIPH_BASE(Container[index].mPeripherie), "Invalid");
        static_assert(IS_I2C_DIGITAL_FILTER(0x3), "Invalid Filter!"); // TODO compare with which value?
        static_assert(IS_I2C_CLOCK_SPEED(Container[index].mConfiguration.I2C_ClockSpeed), "Invalid clock speed!");
        static_assert(IS_I2C_MODE(Container[index].mConfiguration.I2C_Mode), "Invalid");
        static_assert(IS_I2C_DUTY_CYCLE(Container[index].mConfiguration.I2C_DutyCycle), "Invalid duty cycle");
        static_assert(IS_I2C_ACK_STATE(Container[index].mConfiguration.I2C_Ack), "Invalid");
        static_assert(IS_I2C_ACKNOWLEDGE_ADDRESS(Container[index].mConfiguration.I2C_AcknowledgedAddress), "Invalid");
        static_assert(IS_I2C_OWN_ADDRESS1(Container[index].mConfiguration.I2C_OwnAddress1), "Invalid");

        static_assert(index != I2c::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_I2C_H_ */
