// SPDX-License-Identifier: GPL-3.0
/*
 * Copyright (c) 2014-2018 Nils Weiss
 */
#ifndef SOURCES_PMD_USARTWITHDMA_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_USARTWITHDMA_CONFIG_DESCRIPTION_H_

static constexpr const uint8_t NUMBER_OF_INSTANCES = 1;

#else
#ifndef SOURCES_PMD_USARTWITHDMA_CONFIG_CONTAINER_H_
#define SOURCES_PMD_USARTWITHDMA_CONFIG_CONTAINER_H_

static constexpr const std::array<const UsartWithDma, UsartWithDma::NUMBER_OF_INSTANCES> Container =
{ {
      UsartWithDma(Factory<Usart>::get<Usart::DISCO_DEMO_COM>(), USART_DMAReq_Rx | USART_DMAReq_Tx,
                   &Factory<Dma>::get<Dma::DISCO_DEMO_COM_TX>(), &Factory<Dma>::get<Dma::DISCO_DEMO_COM_RX>())
  } };

#endif /* SOURCES_PMD_USART_CONFIG_CONTAINER_H_ */
#endif // SOURCES_PMD_USARTWITHDMA_CONFIG_DESCRIPTION_H_
