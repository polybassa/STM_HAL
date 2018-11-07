// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */

#ifndef SOURCES_PMD_USARTWITHDMA_CONFIG_CONTAINER_H_
#define SOURCES_PMD_USARTWITHDMA_CONFIG_CONTAINER_H_

static const size_t CONTAINERSIZE = 3;

static constexpr const std::array<const UsartWithDma, CONTAINERSIZE> Container =
{ {
      UsartWithDma(Factory<Usart>::get<Usart::DEBUG_IF>(), USART_DMAReq_Tx,
                   &Factory<Dma>::get<Dma::USART1_TX>(), nullptr),
      UsartWithDma(Factory<Usart>::get<Usart::SECCO_COM>(), USART_DMAReq_Tx,
                   &Factory<Dma>::get<Dma::USART2_TX>(), nullptr),
      UsartWithDma(Factory<Usart>::get<Usart::MODEM_COM>(), USART_DMAReq_Tx,
                   &Factory<Dma>::get<Dma::USART3_TX>(), nullptr)
  } };

#endif /* SOURCES_PMD_USART_CONFIG_CONTAINER_H_ */
