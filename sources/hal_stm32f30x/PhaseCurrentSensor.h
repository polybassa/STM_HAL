// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_PHASE_CURRENT_SENSOR_H_
#define SOURCES_PMD_PHASE_CURRENT_SENSOR_H_

#include <cstdint>
#include <array>
#include "hal_Factory.h"
#include "stm32f30x_syscfg.h"
#include "TimHalfBridge.h"
#include "AdcWithDma.h"

namespace hal
{
struct PhaseCurrentSensor {
#include "PhaseCurrentSensor_config.h"

    PhaseCurrentSensor() = delete;
    PhaseCurrentSensor(const PhaseCurrentSensor&) = delete;
    PhaseCurrentSensor(PhaseCurrentSensor&&) = default;
    PhaseCurrentSensor& operator=(const PhaseCurrentSensor&) = delete;
    PhaseCurrentSensor& operator=(PhaseCurrentSensor&&) = delete;

    float getPhaseCurrent(void) const;
    float getCurrentVoltage(void) const;
    void registerValueAvailableSemaphore(os::Semaphore* valueAvailable) const;
    void unregisterValueAvailableSemaphore(void) const;
    void calibrate(void) const;
    void reset(void) const;
    void setPulsWidthForTriggerPerMill(uint32_t) const;
    void setNumberOfMeasurementsForPhaseCurrentValue(uint32_t) const;
    void enable(void) const;
    void disable(void) const;
    size_t getNumberOfMeasurementsForPhaseCurrentValue(void) const;

private:
    constexpr PhaseCurrentSensor(const enum Description&  desc,
                                 const HalfBridge&        hBridge,
                                 const AdcWithDma&        adc,
                                 const TIM_OCInitTypeDef& adcTrgoConf) :
        mDescription(desc), mHBridge(hBridge), mAdcWithDma(adc), mAdcTrgoConfiguration(adcTrgoConf){}

    void updateCurrentValue(void) const;
    void initialize(void) const;

    const enum Description mDescription;
    const HalfBridge& mHBridge;
    const AdcWithDma& mAdcWithDma;
    const TIM_OCInitTypeDef mAdcTrgoConfiguration;

    mutable float mPhaseCurrentValue = 0;
    mutable uint16_t mOffsetValue = 2000;
    mutable size_t mNumberOfMeasurementsForPhaseCurrentValue = MAX_NUMBER_OF_MEASUREMENTS;

    mutable os::Semaphore* mValueAvailableSemaphore = nullptr;

    friend class Factory<PhaseCurrentSensor>;

    static std::array<
                      std::array<uint16_t, MAX_NUMBER_OF_MEASUREMENTS>,
                      Description::__ENUM__SIZE> MeasurementValueBuffer;
};

template<>
class Factory<PhaseCurrentSensor>
{
#include "PhaseCurrentSensor_config.h"

    Factory(void)
    {
        for (const auto& obj : Container) {
            obj.initialize();
        }

        // if TIM20 provides trigger signal for ADC34, this trigger has to be remaped
        SYSCFG_ADCTriggerRemapConfig(REMAPADCTRIGGER_ADC34_EXT5, ENABLE);
    }
public:

    template<enum PhaseCurrentSensor::Description index>
    static constexpr const PhaseCurrentSensor& get(void)
    {
        static_assert(Container[index].mHBridge.mDescription != hal::HalfBridge::Description::__ENUM__SIZE,
                      "Invalid Tim Object");
        static_assert(index != PhaseCurrentSensor::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_PHASE_CURRENT_SENSOR_H_ */
