/// @file FlashAsEEPROM.h
/// @brief STM32F4 EEPROM Emulation abstraction and corresponding hal::Factory.
/// @author Henning Mende (henning@my-urmo.com)
/// @date   Jun 12, 2020
/// @copyright UrmO GmbH
///
/// This program is free software: you can redistribute it and/or modify it under the terms
/// of the GNU General Public License as published by the Free Software Foundation, either
/// version 3 of the License, or (at your option) any later version.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
/// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
/// See the GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License along with this program.
/// If not, see <https://www.gnu.org/licenses/>.
///
#ifndef SOURCES_HAL_STM32F4XX_FLASHASEEPROM_H_
#define SOURCES_HAL_STM32F4XX_FLASHASEEPROM_H_

#include "hal_Factory.h"
#include "stm32f4xx_flash.h"
#include <array>
extern "C" {
#include "eeprom.h"
}

namespace hal
{
/// @brief STM32F4 EEPROM Emulation abstraction for usage with C++.
///
/// This class is based on the STM32F4xx EEPROM Emulation library from ST-Microelectronics.
/// It allows to store several uint16_t values in flash memory, while reducing the total
/// erase cycles for the utilized flash pages. For further information read the application
/// note: AN 3969.
/// @note The methods of this class should never be used in real-time critical tasks. Additionally
/// some interrupts are disabled during read and write operations.
class FlashAsEeprom
{
public:
#include "FlashAsEeprom_config.h"

    FlashAsEeprom() = delete;
    FlashAsEeprom(const FlashAsEeprom&) = delete;
    FlashAsEeprom(FlashAsEeprom&&) = default;
    FlashAsEeprom& operator=(const FlashAsEeprom&) = delete;
    FlashAsEeprom& operator=(FlashAsEeprom&&) = delete;

    /// Read stored value from flash.
    /// @param[out] status True if success.
    /// @return Value for this variable.
    uint16_t read(bool& status) const;

    /// Store value in flash.
    /// @param data Value to store.
    /// @return True if success.
    bool write(const uint16_t data) const;

private:
    /// Private constructor for static factory pattern.
    /// @param desc Identifier for this instance (corresponds to virtual address).
    constexpr FlashAsEeprom(const Description& desc) : mDescription(desc) {}

    /// Identifier of this class instance.
    const Description mDescription;

    friend class Factory<FlashAsEeprom>;
};

/// FlashAsEeprom specialization of hal::Factory.
template<>
class Factory<FlashAsEeprom>
{
public:
    /// Returns the desired FlashAsEeprom instance.
    /// @tparam index Identifier of the desired instance.
    /// @return Constant reference to the instance.
    template<enum FlashAsEeprom::Description index>
    static constexpr const FlashAsEeprom& get(void)
    {
        // general factory initialization checks
        static_assert(index != FlashAsEeprom::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        // specific FlashAsEeprom initialization checks
        static_assert(FlashAsEeprom::__ENUM__SIZE == NB_OF_VAR,
                      "Number of variables in eeprom.h and the number of entries in Description in "
                      "FlashAsEeprom_config.h must be equal");
        static_assert(Container[index].mDescription != 0xFFFF, "Value 0xFFFF not allowed for virtual address");

        // eeprom.h configuration
        static_assert(IS_FLASH_SECTOR(PAGE0_ID), "No valid flash sector for page 0");
        static_assert(IS_FLASH_SECTOR(PAGE1_ID), "No valid flash sector for page 1");
        static_assert(IS_VOLTAGERANGE(VOLTAGE_RANGE), "Invalid voltage Range");
        static_assert(VOLTAGE_RANGE != VoltageRange_1, "Voltage range too low, for FLASH_ProgramHalfWord function");

        static_assert(IS_FLASH_ADDRESS(EEPROM_START_ADDRESS), "EEPROM_START_ADDRESS is not in flash region");
        static_assert(IS_FLASH_ADDRESS(PAGE0_BASE_ADDRESS), "PAGE0_BASE_ADDRESS is not in flash region");
        static_assert(IS_FLASH_ADDRESS(PAGE0_END_ADDRESS), "PAGE0_END_ADDRESS is not in flash region");
        static_assert(IS_FLASH_ADDRESS(PAGE1_BASE_ADDRESS), "PAGE1_BASE_ADDRESS is not in flash region");
        static_assert(IS_FLASH_ADDRESS(PAGE1_END_ADDRESS), "PAGE1_END_ADDRESS is not in flash region");
        static_assert(EEPROM_START_ADDRESS == PAGE0_BASE_ADDRESS || EEPROM_START_ADDRESS == PAGE1_BASE_ADDRESS,
                      "Neither page 0 nor page 1 start at EEPROM_START_ADDRESS");
        static_assert(PAGE0_BASE_ADDRESS + PAGE_SIZE - 1 == PAGE0_END_ADDRESS,
                      "Page 0 start and end addresses don't match page size");
        static_assert(PAGE1_BASE_ADDRESS + PAGE_SIZE - 1 == PAGE1_END_ADDRESS,
                      "Page 0 start and end addresses don't match page size");

        return Container[index];
    }

private:
#include "FlashAsEeprom_config.h"
    /// Default constructor initializes the eeprom and flash environment.
    Factory(void);

    template<typename U>
    friend const U& getFactory(void);
};
} // namespace hal

#endif // SOURCES_HAL_STM32F4XX_FLASHASEEPROM_H_
