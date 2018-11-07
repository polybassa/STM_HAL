// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

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
