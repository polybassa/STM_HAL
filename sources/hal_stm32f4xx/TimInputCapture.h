/// @file TimInputCapture.h
/// @brief Timer input capture abstraction and corresponding hal::Factory.
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
#ifndef SOURCES_HAL_STM32F4XX_TIMINPUTCAPTURE_H_
#define SOURCES_HAL_STM32F4XX_TIMINPUTCAPTURE_H_

#include "Tim.h"

namespace hal
{
/// Abstraction for the input capture functionality of the stm timers.
class TimInputCapture
{
public:
#include "TimInputCapture_config.h"

    /// Counter triggers
    enum TriggerSelection {
        ITR0 = TIM_TS_ITR0,      //!< Internal trigger 0
        ITR1 = TIM_TS_ITR1,      //!< Internal trigger 1
        ITR2 = TIM_TS_ITR2,      //!< Internal trigger 2
        ITR3 = TIM_TS_ITR3,      //!< Internal trigger 3
        TI1F_ED = TIM_TS_TI1F_ED,//!< Timer input edge detector
        TI1FP1 = TIM_TS_TI1FP1,  //!< Filtered timer input 1
        TI2FP2 = TIM_TS_TI2FP2,  //!< Filtered timer input 1
        ETRF = TIM_TS_ETRF       //!< External trigger input
    };

    /// Timer interrupt flags.
    enum Flags {
        UPDATE = TIM_FLAG_Update,              //!< Timer counter value reset.
        CC1 = TIM_FLAG_CC1,                    //!< Capture Compare Channel 1.
        CC2 = TIM_FLAG_CC2,                    //!< Capture Compare Channel 2.
        CC3 = TIM_FLAG_CC3,                    //!< Capture Compare Channel 3.
        CC4 = TIM_FLAG_CC4,                    //!< Capture Compare Channel 4.
        COM = TIM_FLAG_COM,                    //!< Commutation event flag.
        TRIGGER = TIM_FLAG_Trigger,            //!< Trigger event detected.
        BREAK = TIM_FLAG_Break,                //!< Break input detected.
        CC1OF_OVER_CAPTURE_1 = TIM_FLAG_CC1OF, //!< Capture Compare 1 over capture.
        CC2OF_OVER_CAPTURE_2 = TIM_FLAG_CC2OF, //!< Capture Compare 2 over capture.
        CC3OF_OVER_CAPTURE_3 = TIM_FLAG_CC3OF, //!< Capture Compare 3 over capture.
        CC4OF_OVER_CAPTURE_4 = TIM_FLAG_CC4OF  //!< Capture Compare 4 over capture.
    };

    /// Enables the capture functionality for this channel
    void enable(void) const;

    /// Disables the capture functionality for this channel
    void disable(void) const;

    /// @brief Changes the timer input trigger to @a source.
    /// @param source Trigger source for the base timer.
    /// @note Affects the base timer, not only this input channel.
    void selectInputTrigger(const TriggerSelection source) const;

    /// Sets the input trigger according to this TimInputCapture configuration.
    inline void prepareMeasurement(void) const
    {
        selectInputTrigger(mTriggerSource);
    }

    /// @brief Returns the last captured value.
    ///
    /// Most of the time the return value is 16 bits long, but for timer 2 and 5 it
    /// has 32 bits.
    /// @return Last captured value.
    uint32_t getCapture(void) const;

    /// Returns all pending flags of the base timer.
    ///
    /// The flags are shared for all
    /// @return Mask with all pending timer flags.
    uint16_t getFlags(void) const;

    /// @brief Checks whether a flag is set.
    /// @param flag Flag to check for.
    /// @return True if @a flag is set.
    bool isFlagSet(const Flags flag) const;

    /// @brief Returns whether a new value was captured since the last reading.
    /// @return@return True if a new value is available.
    bool isNewCaptureAvailable(void) const;

    /// @brief Was more than one capture finished since the last reading?
    /// @param clearFlag Set to true, to reset the flag after readout.
    /// @return True if an over capture event occurred.
    bool isOverCaptured(const bool clearFlag = true) const;

    /// Clears a specific timer flag.
    /// @param flag Flag(s) to clear.
    inline void clearFlag(const uint16_t flag) const
    {
        mTim.clearPendingInterruptFlag(flag);
    }

private:
    /// @brief Private constructor for the static factory pattern.
    /// @param desc Identifier of the class instance.
    /// @param baseTim Timer based used for the input channel.
    /// @param config Input channel configuration.
    /// @param trigger Input trigger for the timer to be used with this capture channel.
    constexpr TimInputCapture(const enum Description&  desc,
                              const Tim&               baseTim,
                              const TIM_ICInitTypeDef& config,
                              const TriggerSelection&  trigger) :
        mDescription(desc),
        mTim(baseTim),
        mConfiguration(config),
        mTriggerSource(trigger){}

    /// Instance description for the class instance.
    const Description mDescription;

    /// Timer base for this capture channel
    const Tim& mTim;

    /// Channel configuration
    const TIM_ICInitTypeDef mConfiguration;

    /// Input trigger for the timer, connected to this capture channel.
    const TriggerSelection mTriggerSource;

    /// Initializes hardware to capture input frequencies.
    void initialize(void) const;

    friend class Factory<TimInputCapture>;
};

/// TimInputCapture specialization of the hal::Factory.
template<>
class Factory<TimInputCapture>
{
public:
    /// Returns a specific TimInputCapture instance.
    /// @tparam index Identifier of the desired instance.
    /// @return Const reference to the specific instance.
    template<enum TimInputCapture::Description index>
    static constexpr const TimInputCapture& get(void)
    {
        static_assert(IS_TIM_CHANNEL(Container[index].mConfiguration.TIM_Channel), "Invalid Channel!");
        static_assert(IS_TIM_IC_POLARITY(Container[index].mConfiguration.TIM_ICPolarity), "Invalid Polarity!");
        static_assert(IS_TIM_IC_SELECTION(Container[index].mConfiguration.TIM_ICSelection),
                      "Invalid capture selection!");
        static_assert(IS_TIM_IC_PRESCALER(Container[index].mConfiguration.TIM_ICPrescaler), "Invalid prescaler!");
        static_assert(Container[index].mConfiguration.TIM_ICFilter >= 0, "Filter value too low!");
        static_assert(Container[index].mConfiguration.TIM_ICFilter <= 0xF, "Filter value too high!");

        static_assert(Container[index].mTim.mDescription != Tim::Description::__ENUM__SIZE, "Invalid Tim Object");

        static_assert(index != TimInputCapture::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

private:
#include "TimInputCapture_config.h"
    /// Factory constructor, initializes all instances.
    Factory(void)
    {
        for (const auto& obj : Container) {
            obj.initialize();
        }
    }

    template<typename U>
    friend const U& getFactory(void);
};
} // namespace hal

#endif // SOURCES_HAL_STM32F4XX_TIMINPUTCAPTURE_H_
