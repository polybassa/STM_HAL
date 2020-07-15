/// @file TimInputCapture_config.h
/// @brief Configuration file for the statically allocated hal::TimInputCapture instances.
/// @author Henning Mende (henning@my-urmo.com)
/// @date   Jun 2, 2020
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
#ifndef PROJECT_STM32F4_DISCOVERY_CONFIG_TIMINPUTCAPTURE_DESCRIPTION_
#define PROJECT_STM32F4_DISCOVERY_CONFIG_TIMINPUTCAPTURE_DESCRIPTION_

/// Instance names.
enum Description {
    PERIOD_MEASUREMENT_1,
    PERIOD_MEASUREMENT_2,
    __ENUM__SIZE
};

#else
#ifndef PROJECT_STM32F4_DISCOVERY_CONFIG_TIMINPUTCAPTURE_CONTAINER_
#define PROJECT_STM32F4_DISCOVERY_CONFIG_TIMINPUTCAPTURE_CONTAINER_

/// Configuration and allocation of the hal::TimInputCapture instances.
static constexpr std::array<TimInputCapture, TimInputCapture::__ENUM__SIZE> Container = {
    TimInputCapture(TimInputCapture::PERIOD_MEASUREMENT_1,
                    Factory<Tim>::get<Tim::PERIOD_MEASUREMENT>(),
                    {TIM_Channel_1, TIM_ICPolarity_Falling, TIM_ICSelection_DirectTI, TIM_ICPSC_DIV8, 4},
                    TimInputCapture::TI1FP1),
    TimInputCapture(TimInputCapture::PERIOD_MEASUREMENT_2,
                    Factory<Tim>::get<Tim::PERIOD_MEASUREMENT>(),
                    {TIM_Channel_2, TIM_ICPolarity_Falling, TIM_ICSelection_DirectTI, TIM_ICPSC_DIV8, 4},
                    TimInputCapture::TI2FP2)
};

#endif // PROJECT_STM32F4_DISCOVERY_CONFIG_TIMINPUTCAPTURE_CONTAINER_
#endif // PROJECT_STM32F4_DISCOVERY_CONFIG_TIMINPUTCAPTURE_DESCRIPTION_
