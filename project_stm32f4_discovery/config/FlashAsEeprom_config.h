/// @file FlashAsEeprom_config.h
/// @brief Configuration file for the statically allocated FlashAsEeprom instances.
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
#ifndef PROJECT_STM32F4_DISCOVERY_CONFIG_FLASHASEEPROM_CONFIG_DESCRIPTION_
#define PROJECT_STM32F4_DISCOVERY_CONFIG_FLASHASEEPROM_CONFIG_DESCRIPTION_

/// Instance names.
enum Description:uint16_t {
    RESTART_COUNTER,
    __ENUM__SIZE
};

#else
#ifndef PROJECT_STM32F4_DISCOVERY_CONFIG_FLASHASEEPROM_CONFIG_CONTAINER_
#define PROJECT_STM32F4_DISCOVERY_CONFIG_FLASHASEEPROM_CONFIG_CONTAINER_

constexpr static std::array<FlashAsEeprom, FlashAsEeprom::Description::__ENUM__SIZE> Container {
    FlashAsEeprom(FlashAsEeprom::RESTART_COUNTER)
};

#endif // PROJECT_STM32F4_DISCOVERY_CONFIG_FLASHASEEPROM_CONFIG_CONTAINER_
#endif // PROJECT_STM32F4_DISCOVERY_CONFIG_FLASHASEEPROM_CONFIG_DESCRIPTION_
