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

#ifndef SOURCES_PMD_COMP_H_
#define SOURCES_PMD_COMP_H_

#include <cstdint>
#include <limits>
#include <array>
#include <functional>
#include "stm32f30x_comp.h"
#include "hal_Factory.h"

namespace hal
{
struct Comp {
#include "Comp_config.h"

    const enum Description mDescription;

    Comp() = delete;
    Comp(const Comp&) = delete;
    Comp(Comp &&) = default;
    Comp& operator=(const Comp&) = delete;
    Comp& operator=(Comp &&) = delete;

private:
    constexpr Comp(const enum Description& desc,
                   const uint32_t&         peripherie,
                   const COMP_InitTypeDef& conf) :
        mDescription(desc), mPeripherie(peripherie),
        mConfiguration(conf) {}

    const uint32_t mPeripherie;
    const COMP_InitTypeDef mConfiguration;

    void initialize(void) const;

    friend class Factory<Comp>;
};

template<>
class Factory<Comp>
{
#include "Comp_config.h"

    Factory(void)
    {
        for (const Comp& comp : Container) {
            comp.initialize();
        }
    }

public:
    template<enum Comp::Description index>
    static constexpr const Comp& get(void)
    {
        static_assert(IS_COMP_ALL_PERIPH(Container[index].mPeripherie), "Invalid Peripheries ");
        static_assert(IS_COMP_INVERTING_INPUT(Container[index].mConfiguration.COMP_InvertingInput), "Invalid Parameter");
        static_assert(IS_COMP_NONINVERTING_INPUT(
                                                 Container[index].mConfiguration.COMP_NonInvertingInput),
                      "Invalid Parameter");
        static_assert(IS_COMP_OUTPUT(Container[index].mConfiguration.COMP_Output), "Invalid Parameter");
        static_assert(IS_COMP_BLANKING_SOURCE(Container[index].mConfiguration.COMP_BlankingSrce), "Invalid Parameter ");
        static_assert(IS_COMP_OUTPUT_POL(Container[index].mConfiguration.COMP_OutputPol), "Invalid Parameter");
        static_assert(IS_COMP_HYSTERESIS(Container[index].mConfiguration.COMP_Hysteresis), "Invalid Parameter ");
        static_assert(IS_COMP_MODE(Container[index].mConfiguration.COMP_Mode), "Invalid Parameter ");

        static_assert(index != Comp::Description::__ENUM__SIZE, "__ENUM__SIZE is not accessible");
        static_assert(Container[index].mDescription == index, "Wrong mapping between Description and Container");

        return Container[index];
    }

    template<typename U>
    friend const U& getFactory(void);
};
}

#endif /* SOURCES_PMD_COMP_H_ */
