// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_USARTWITHDMA_CONFIG_CONTAINER_H_
#define SOURCES_PMD_USARTWITHDMA_CONFIG_CONTAINER_H_

static constexpr const std::array<const UsartWithDma, 1> Container =
{ {
      UsartWithDma(Factory<Usart>::get<Usart::MSCOM_IF>(), USART_DMAReq_Rx | USART_DMAReq_Tx,
                   &Factory<Dma>::get<Dma::USART2_TX>(), &Factory<Dma>::get<Dma::USART2_RX>())
  } };

#endif /* SOURCES_PMD_USART_CONFIG_CONTAINER_H_ */
