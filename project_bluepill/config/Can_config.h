// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_CAN_INTERRUPTS_H_
#define SOURCES_CAN_INTERRUPTS_H_

#define USB_LP_CAN1_RX0_INTERRUPT_ENABLED true
#define CAN1_RX1_INTERRUPT_ENABLED true

#endif /* SOURCES_CAN_INTERRUPTS_H_ */

#ifndef SOURCES_CAN_CONFIG_DESCRIPTION_H_
#define SOURCES_CAN_CONFIG_DESCRIPTION_H_

enum Description {
    MAINCAN,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_CAN_CONFIG_CONTAINER_H_
#define SOURCES_CAN_CONFIG_CONTAINER_H_

static constexpr const std::array<const Can, Can::__ENUM__SIZE + 1> Container =
{ {
      Can(Can::MAINCAN,
          CAN1_BASE,
          CAN_InitTypeDef { 9, CAN_Mode_Normal, CAN_SJW_1tq, CAN_BS1_13tq, CAN_BS2_2tq, DISABLE, DISABLE,
                            DISABLE, DISABLE, DISABLE, DISABLE}),
      Can(Can::__ENUM__SIZE, 0, CAN_InitTypeDef { 0, 0, 0, 0, 0, DISABLE, DISABLE,
                                                  DISABLE, DISABLE, DISABLE, DISABLE})
  }};

static constexpr const std::array<const CAN_FilterInitTypeDef, 1> CanFilterContainer =
{ {
      CAN_FilterInitTypeDef{0x0000 << 5, 0x0000, 0x0000 << 5, 0x0000, 0, 0,
                            CAN_FilterMode_IdMask, CAN_FilterScale_32bit, ENABLE}
  }};

#endif /* SOURCES_CAN_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_CAN_CONFIG_DESCRIPTION_H_ */
