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

#ifndef SOURCES_PMD_TIMHALLMETER_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_TIMHALLMETER_CONFIG_DESCRIPTION_H_

enum Description {
    BLDC_METER,
    __ENUM__SIZE
};

static constexpr uint32_t SYSTEMCLOCK = 72000000;
static constexpr uint32_t POLE_PAIRS = 7;

#else
#ifndef SOURCES_PMD_TIMHALLMETER_CONFIG_CONTAINER_H_
#define SOURCES_PMD_TIMHALLMETER_CONFIG_CONTAINER_H_

static constexpr const std::array<const HallMeter, HallMeter::__ENUM__SIZE> Container =
{ {
      HallMeter(HallMeter::BLDC_METER,
                Factory<Tim>::get<Tim::HALL_METER>(),
                TIM_ICInitTypeDef { TIM_Channel_1, TIM_ICPolarity_Rising, TIM_ICSelection_TRC, TIM_ICPSC_DIV1, 0x00}
                )
  } };

#endif /* SOURCES_PMD_TIMHALLMETER_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_TIMHALLMETER_CONFIG_DESCRIPTION_H_ */
