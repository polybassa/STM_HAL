/* Copyright (C) 2015  Nils Weiss
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

#ifndef SOURCES_PMD_DAC_H_
#define SOURCES_PMD_DAC_H_

#include <cstdint>
#include <array>
#include "stm32f30x_dac.h"
#include "stm32f30x_rcc.h"
#include "hal_Factory.h"
#include "Dma.h"

namespace hal
{
struct Dac {
#include "Dac_config.h"

    Dac() = delete;
    Dac(const Dac&) = delete;
    Dac(Dac&&) = default;
    Dac& operator=(const Dac&) = delete;
    Dac& operator=(Dac&&) = delete;

    void set(const uint32_t data) const;
    uint16_t get(void) const;
    void trigger(void) const;

    void enable(void) const;
    void disable(void) const;

private:
    constexpr Dac(const enum Description& desc,
                  const uint32_t&         peripherie,
                  const uint32_t&         channel,
                  const DAC_InitTypeDef&  conf,
                  const uint16_t          align,
                  Dma const* const        dma = nullptr) :
        mDescription(desc),
        mPeripherie(peripherie),
        mChannel(channel),
        mConfiguration(conf),
        mAlign(align),
        mDma(dma) {}

    const enum Description mDescription;
    const uint32_t mPeripherie;
    const uint32_t mChannel;
    const DAC_InitTypeDef mConfiguration;
    const uint16_t mAlign;
    Dma const* const mDma;

    void initialize(void) const;

    void set(const uint32_t data, const uint16_t align) const;

    friend class Factory<Dac>;
};

template<>
class Factory<Dac>
{
#include "Dac_config.h"

    Factory(void)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC1, ENABLE);

        DAC_DeInit(DAC1);

        for (const auto& dac : Container) {
            dac.initialize();
        }
    }
public:

    template<enum Dac::Description index>
    static constexpr const Dac& get(void)
    {
        static_assert(IS_DAC_ALL_PERIPH_BASE(Container[index].mPeripherie), "Invalid Peripheries ");
        static_assert(Container[index].mDescription < Dac::Description::__ENUM__SIZE, "Invalid Parameter");

        static_assert(IS_DAC_CHANNEL(Container[index].mChannel), "Invalid Parameter");
        static_assert(IS_DAC_BUFFER_SWITCH_STATE(
                                                 Container[index].mConfiguration.DAC_Buffer_Switch),
                      "Invalid Parameter");
        static_assert(IS_DAC_TRIGGER(Container[index].mConfiguration.DAC_Trigger), "Invalid Parameter");
        static_assert(IS_DAC_GENERATE_WAVE(Container[index].mConfiguration.DAC_WaveGeneration), "Invalid Parameter");
        static_assert(IS_DAC_LFSR_UNMASK_TRIANGLE_AMPLITUDE(Container[index].mConfiguration.
                                                            DAC_LFSRUnmask_TriangleAmplitude), "Invalid Parameter");

        static_assert(IS_DAC_ALIGN(Container[index].mAlign), "Invalid Parameter");

        static_assert(index != Dac::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }
    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_DAC_H_ */
