/// @file SoftwareSpi.cpp
/// @brief Software spi module and corresponding hal::Factory (implementation).
/// @author Henning Mende (henning@my-urmo.com)
/// @date   Oct 1, 2019
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
#include <SoftwareSpi.h>

namespace hal
{
void SoftwareSpi::send(const uint16_t data) const
{
    readyToReceiveFlags[mDescription] = false;
    rxBuffers[mDescription] = 0;
    uint16_t position = 0;

    wait();

    for (int i = 0; i < 16; i++) {
        if (mMsbFirst) {
            position = (15 - i);
        } else {
            position = i;
        }
        const bool output = (data & (1 << position)) != 0;

        risingEdge(output, position);
        wait();
        fallingEdge(output, position);
        wait();
    }
    readyToReceiveFlags[mDescription] = true;
}

bool SoftwareSpi::isReadyToReceive(void) const
{
    return readyToReceiveFlags[mDescription];
}

uint16_t SoftwareSpi::receive(void) const
{
    readyToReceiveFlags[mDescription] = false;
    return rxBuffers[mDescription];
}

void SoftwareSpi::wait(void) const
{
    for (volatile size_t i = 0; i < mClockInterval / 2; i++) {
        // do nothing...wait
    }
}

void SoftwareSpi::risingEdge(const bool output, const uint16_t position) const
{
    if (mWriteOnRisingEdge) {
        mMosiPin = output;
    }

    mClockPin = true;
    wait();

    if (mReadOnRisingEdge) {
        rxBuffers[mDescription] |= static_cast<bool>(mMisoPin) << position;
    }
}

void SoftwareSpi::fallingEdge(const bool output, uint16_t position) const
{
    if (!mWriteOnRisingEdge) {
        mMosiPin = output;
    }

    mClockPin = false;
    wait();

    if (!mReadOnRisingEdge) {
        rxBuffers[mDescription] |= static_cast<bool>(mMisoPin) << position;
    }
}

constexpr const std::array<const SoftwareSpi, SoftwareSpi::__ENUM__SIZE> Factory<SoftwareSpi>::Container;
std::array<bool, SoftwareSpi::__ENUM__SIZE> SoftwareSpi::readyToReceiveFlags;
std::array<uint16_t, SoftwareSpi::__ENUM__SIZE> SoftwareSpi::rxBuffers;
} // namespace hal
