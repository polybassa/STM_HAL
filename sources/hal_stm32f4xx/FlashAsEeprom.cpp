/// @file FlashAsEEPROM.cpp
/// @brief STM32F4 EEPROM Emulation abstraction and corresponding hal::Factory (implementation).
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
#include <FlashAsEeprom.h>
#include "trace.h"

/* GLOBAL VARIABLES */
static const int __attribute__((used)) g_DebugZones = ZONE_ERROR | ZONE_WARNING |
                                                      ZONE_VERBOSE | ZONE_INFO;

/// Storage for the virtual addresses of the EEPROM abstraction.
uint16_t VirtAddVarTab[hal::FlashAsEeprom::__ENUM__SIZE];

namespace hal
{
constexpr std::array<FlashAsEeprom, FlashAsEeprom::Description::__ENUM__SIZE> Factory<FlashAsEeprom>::Container;

uint16_t FlashAsEeprom::read(bool& status) const
{
    uint16_t data;
    const uint16_t errorCode = EE_ReadVariable(static_cast<uint16_t>(mDescription), &data);

    if (errorCode == 0) {
        status = true;
    } else {
        status = false;
        Trace(ZONE_VERBOSE, "errorCode: %d\n", errorCode);
    }

    return data;
}

bool FlashAsEeprom::write(const uint16_t data) const
{
    const uint16_t errorCode = EE_WriteVariable(static_cast<uint16_t>(mDescription), data);
    const bool success = errorCode == FLASH_COMPLETE;

    if (!success) {
        Trace(ZONE_VERBOSE, "errorCode: %d\n", errorCode);
    }
    return success;
}

Factory<FlashAsEeprom>::Factory(void)
{
    FLASH_Unlock();
    EE_Init();

    // initialize virtual addresses.
    for (uint16_t index = 0; index < FlashAsEeprom::__ENUM__SIZE && index < 0xFFFF; index++) {
        VirtAddVarTab[index] = index;
    }
}
} // namespace hal
