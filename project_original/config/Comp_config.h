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

#ifndef SOURCES_PMD_COMP_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_COMP_CONFIG_DESCRIPTION_H_

enum Description {
    COMP_3,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_COMP_CONFIG_CONTAINER_H_
#define SOURCES_PMD_COMP_CONFIG_CONTAINER_H_

static constexpr const std::array<const Comp, Comp::__ENUM__SIZE> Container =
{ {
      Comp(Comp::COMP_3,
           COMP_Selection_COMP3,
           COMP_InitTypeDef { COMP_InvertingInput_IO1, COMP_NonInvertingInput_IO1, COMP_Output_None,
                              COMP_BlankingSrce_None,
                              COMP_OutputPol_NonInverted, COMP_Hysteresis_No,
                              COMP_Mode_HighSpeed })
  }};

#endif /* SOURCES_PMD_COMP_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_COMP_CONFIG_DESCRIPTION_H_ */
