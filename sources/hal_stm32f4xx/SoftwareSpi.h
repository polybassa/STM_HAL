/// @file SoftwareSpi.h
/// @brief Software spi module and corresponding hal::Factory.
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
#ifndef SOURCES_HAL_STM32F4XX_SOFTWARESPI_H_
#define SOURCES_HAL_STM32F4XX_SOFTWARESPI_H_

#include "hal_Factory.h"
#include "Gpio.h"

namespace hal
{
/// @brief Spi implemented in Software, with usage of input/output Pins.
///
/// Used Abbreviations
/// Abbr | Meaning:
/// ---- | -------------------
/// MoSi | Master out Slave in
/// MiSo | Master in Slave out
class SoftwareSpi
{
public:
#include "SoftwareSpi_config.h"
    SoftwareSpi() = delete;
    SoftwareSpi(const SoftwareSpi&) = delete;
    SoftwareSpi(SoftwareSpi&&) = default;
    SoftwareSpi& operator=(const SoftwareSpi&) = delete;
    SoftwareSpi& operator=(SoftwareSpi&&) = delete;

    /// @brief Sends 16 bits.
    /// @param Data to send.
    void send(const uint16_t data) const;

    /// @brief Indicates whether new data is in the receive buffer.
    /// @return True if the receive Buffer contains 16 bit.
    bool isReadyToReceive(void) const;

    /// @brief Returns the content of the receive buffer.
    ///
    /// The isReadyToReceive flag is reset by a call to this method.
    /// @return Content of the receive Buffer.
    uint16_t receive(void) const;

private:
    /// @brief Private constructor used by @ref hal::Factory.
    ///
    /// All provided Gpios must be configured as output before calling any methods of this
    /// class.
    ///
    /// @param desc     Identifier of the class instance.
    /// @param clockPin Gpio used for the spi clock signal.
    /// @param mosiPin  Gpio used for the spi MoSi signal.
    /// @param misoPin  Gpio used for the spi MiSo signal.
    /// @param clockInterval     Counter value to adjust the timing of the clock.
    /// @param readOnRisingEdge  Specifies the clock edge to read on.
    /// @param writeOnRisingEdge Specifies the clock edge to write on.
    /// @param msbFirst          Specifies the data direction.
    constexpr SoftwareSpi(const Description& desc,
                          const hal::Gpio&   clockPin,
                          const hal::Gpio&   mosiPin,
                          const hal::Gpio&   misoPin,
                          const size_t       clockInterval = 2,
                          const bool         readOnRisingEdge = true,
                          const bool         writeOnRisingEdge = false,
                          const bool         msbFirst = true) :
        mDescription(desc),
        mClockPin(clockPin),
        mMosiPin(mosiPin),
        mMisoPin(misoPin),
        mClockInterval(clockInterval),
        mReadOnRisingEdge(readOnRisingEdge),
        mWriteOnRisingEdge(writeOnRisingEdge),
        mMsbFirst(msbFirst)
    {}

    /// Receive buffers for all class instances.
    static std::array<uint16_t, __ENUM__SIZE> rxBuffers;

    /// Receive flags for all class instances.
    static std::array<bool, __ENUM__SIZE> readyToReceiveFlags;

    /// Instance description for factory pattern.
    const enum Description mDescription;
    /// Gpio for the clock signal.
    const hal::Gpio& mClockPin;
    /// Gpio for the MoSi signal.
    const hal::Gpio& mMosiPin;
    /// Gpio for the MiSo signal.
    const hal::Gpio& mMisoPin;
    /// Counter value for the clock timing.
    const size_t mClockInterval;
    /// Specifier of the clock edge to read on.
    const bool mReadOnRisingEdge;
    /// Specifier of the clock edge to write on.
    const bool mWriteOnRisingEdge;
    /// Specifier for the data direction.
    const bool mMsbFirst;

    /// @brief Executes the actions for a rising edge of a clock pulse.
    ///
    /// The output is only set on the MoSi pin if writeOnRisingEdge is true.
    ///
    /// @param output   True for high MoSi signal.
    /// @param position Number of the Bit (clock pulse).
    void risingEdge(const bool output, const uint16_t position) const;

    /// @brief Executes the actions for a falling edge of a clock pulse.
    ///
    /// The output is only set on the MoSi pin if writeOnRisingEdge is false.
    ///
    /// @param output   True for high MoSi signal.
    /// @param position Number of the Bit (clock pulse).
    void fallingEdge(const bool output, const uint16_t position) const;

    /// @brief Waits for a half clock pulse.
    void wait(void) const;

    friend class Factory<SoftwareSpi>;
};

/// SoftwareSpi specialization of the hal::Factory.
template<>
class Factory<SoftwareSpi>
{
#include "SoftwareSpi_config.h"

    Factory(void) {}

public:
    /// @brief Returns a specific SoftwareSpi instance.
    /// @tparam index Identifier of the instance.
    /// @return Specific SoftwareSpi instance.
    template<enum SoftwareSpi::Description index>
    static constexpr const SoftwareSpi& get(void)
    {
        static_assert(index != SoftwareSpi::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        // check GPIO mode is input output, or gpio is reconfigurable
        static_assert((Container[index].mClockPin.getModeFromConfig() == GPIO_Mode_OUT) ||
                      Container[index].mClockPin.isReconfigurable(), "Pin must be mode out or reconfigurable!");
        static_assert((Container[index].mMosiPin.getModeFromConfig() == GPIO_Mode_OUT) ||
                      Container[index].mMosiPin.isReconfigurable(), "Pin must be mode out or reconfigurable!");
        static_assert((Container[index].mMisoPin.getModeFromConfig() == GPIO_Mode_IN) ||
                      Container[index].mMisoPin.isReconfigurable(), "Pin must be mode out or reconfigurable!");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
} // namespace hal

#endif // SOURCES_HAL_STM32F4XX_SOFTWARESPI_H_
