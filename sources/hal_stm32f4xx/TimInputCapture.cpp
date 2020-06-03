/// @file TimInputCapture.cpp
/// @brief Timer input capture abstraction and corresponding hal::Factory (implementation).
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
#include "TimInputCapture.h"

namespace hal
{
void TimInputCapture::enable(void) const
{
    TIM_CCxCmd(mTim.getBasePointer(), mConfiguration.TIM_Channel, ENABLE);
    selectInputTrigger(mTriggerSource);
}

void TimInputCapture::disable(void) const
{
    TIM_CCxCmd(mTim.getBasePointer(), mConfiguration.TIM_Channel, DISABLE);
}

void TimInputCapture::selectInputTrigger(const TriggerSelection source) const
{
    TIM_SelectInputTrigger(mTim.getBasePointer(), static_cast<uint16_t>(source));
}

uint32_t TimInputCapture::getCapture(void) const
{
    uint32_t capture = 0;

    switch (mConfiguration.TIM_Channel) {
    case TIM_Channel_1:
        capture = TIM_GetCapture1(mTim.getBasePointer());
        break;

    case TIM_Channel_2:
        capture = TIM_GetCapture2(mTim.getBasePointer());
        break;

    case TIM_Channel_3:
        capture = TIM_GetCapture3(mTim.getBasePointer());
        break;

    case TIM_Channel_4:
        capture = TIM_GetCapture4(mTim.getBasePointer());
        break;
    }

    return capture;
}

uint16_t TimInputCapture::getFlags(void) const
{
    return mTim.getBasePointer()->SR;
}

bool TimInputCapture::isFlagSet(const Flags flag) const
{
    return TIM_GetFlagStatus(mTim.getBasePointer(), static_cast<uint16_t>(flag)) != RESET;
}

bool TimInputCapture::isNewCaptureAvailable(void) const
{
    Flags flag;

    switch (mConfiguration.TIM_Channel) {
    case TIM_Channel_1:
        flag = Flags::CC1;
        break;

    case TIM_Channel_2:
        flag = Flags::CC2;
        break;

    case TIM_Channel_3:
        flag = Flags::CC3;
        break;

    case TIM_Channel_4:
        flag = Flags::CC4;
        break;
    }

    return isFlagSet(flag);
}

bool TimInputCapture::isOverCaptured(const bool clear) const
{
    Flags flag;

    switch (mConfiguration.TIM_Channel) {
    case TIM_Channel_1:
        flag = Flags::CC1OF_OVER_CAPTURE_1;
        break;

    case TIM_Channel_2:
        flag = Flags::CC2OF_OVER_CAPTURE_2;
        break;

    case TIM_Channel_3:
        flag = Flags::CC3OF_OVER_CAPTURE_3;
        break;

    case TIM_Channel_4:
        flag = Flags::CC4OF_OVER_CAPTURE_4;
        break;
    }

    bool status = isFlagSet(flag);

    if (clear) {
        clearFlag(flag);
    }

    return status;
}

void TimInputCapture::initialize(void) const
{
    TIM_ICInit(mTim.getBasePointer(), &mConfiguration);

    TIM_SelectSlaveMode(mTim.getBasePointer(), TIM_SlaveMode_Reset);

    mTim.enable();
}

constexpr std::array<TimInputCapture, TimInputCapture::__ENUM__SIZE> hal::Factory<TimInputCapture>::Container;
} // namespace hal
