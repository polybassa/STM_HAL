/* Copyright (C) 2018  Henning Mende
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Created on: Nov 9, 2018
 *      Author: Henning Mende
 */

#ifndef SOURCES_PMD_NVIC_CONFIG_DESCRIPTION_H_
#define SOURCES_PMD_NVIC_CONFIG_DESCRIPTION_H_

enum Description {
    USER_BUTTON_EXTI,
    ADC_INTERRUPT,
    ADC1_DMA_INT,
    DISCO_DEMO_COM,
    DEBUG_IF,
    USART4_DMA_RX,
    USART4_DMA_TX,
    __ENUM__SIZE
};

#else
#ifndef SOURCES_PMD_NVIC_CONFIG_CONTAINER_H_
#define SOURCES_PMD_NVIC_CONFIG_CONTAINER_H_

static constexpr const std::array<const Nvic, Nvic::__ENUM__SIZE + 1> Container =
{ {
      Nvic(Nvic::USER_BUTTON_EXTI, IRQn::EXTI0_IRQn),
      Nvic(Nvic::ADC_INTERRUPT, IRQn::ADC_IRQn),
      Nvic(Nvic::ADC1_DMA_INT, IRQn::DMA2_Stream0_IRQn),
      Nvic(Nvic::DISCO_DEMO_COM, IRQn::UART4_IRQn),
      Nvic(Nvic::DEBUG_IF, IRQn::USART3_IRQn),
      Nvic(Nvic::USART4_DMA_RX, IRQn::DMA1_Stream2_IRQn),
      Nvic(Nvic::USART4_DMA_TX, IRQn::DMA1_Stream4_IRQn),
      Nvic(Nvic::__ENUM__SIZE, IRQn::FPU_IRQn)
  }};
#endif /* SOURCES_PMD_NVIC_CONFIG_CONTAINER_H_ */
#endif /* SOURCES_PMD_NVIC_CONFIG_DESCRIPTION_H_ */

// ******************* Activation for all interrupt service routines *******************************************
#ifndef PROJECT_CONFIG_NVIC_CONFIG_H_
#define PROJECT_CONFIG_NVIC_CONFIG_H_
/******  Cortex-M4 Processor Exceptions Numbers ****************************************************************/
#define NonMaskableInt_IRQn_ENABLE false   /*!< 2 Non Maskable Interrupt                                          */
#define MemoryManagement_IRQn_ENABLE false   /*!< 4 Cortex-M4 Memory Management Interrupt                           */
#define BusFault_IRQn_ENABLE false   /*!< 5 Cortex-M4 Bus Fault Interrupt                                   */
#define UsageFault_IRQn_ENABLE false   /*!< 6 Cortex-M4 Usage Fault Interrupt                                 */
#define SVCall_IRQn_ENABLE false   /*!< 11 Cortex-M4 SV Call Interrupt                                    */
#define DebugMonitor_IRQn_ENABLE false   /*!< 12 Cortex-M4 Debug Monitor Interrupt                              */
#define PendSV_IRQn_ENABLE false   /*!< 14 Cortex-M4 Pend SV Interrupt                                    */
#define SysTick_IRQn_ENABLE false   /*!< 15 Cortex-M4 System Tick Interrupt                                */
/******  STM32 specific Interrupt Numbers **********************************************************************/
#define WWDG_IRQn_ENABLE false    /*!< Window WatchDog Interrupt                                         */
#define PVD_IRQn_ENABLE false    /*!< PVD through EXTI Line detection Interrupt                         */
#define TAMP_STAMP_IRQn_ENABLE false    /*!< Tamper and TimeStamp interrupts through the EXTI line             */
#define RTC_WKUP_IRQn_ENABLE false    /*!< RTC Wakeup interrupt through the EXTI line                        */
#define FLASH_IRQn_ENABLE false    /*!< FLASH global Interrupt                                            */
#define RCC_IRQn_ENABLE false    /*!< RCC global Interrupt                                              */
#define EXTI0_IRQn_ENABLE true    /*!< EXTI Line0 Interrupt                                              */
#define EXTI1_IRQn_ENABLE false    /*!< EXTI Line1 Interrupt                                              */
#define EXTI2_IRQn_ENABLE false    /*!< EXTI Line2 Interrupt                                              */
#define EXTI3_IRQn_ENABLE false    /*!< EXTI Line3 Interrupt                                              */
#define EXTI4_IRQn_ENABLE false    /*!< EXTI Line4 Interrupt                                              */
#define DMA1_Stream0_IRQn_ENABLE false    /*!< DMA1 Stream 0 global Interrupt                                    */
#define DMA1_Stream1_IRQn_ENABLE false    /*!< DMA1 Stream 1 global Interrupt                                    */
#define DMA1_Stream2_IRQn_ENABLE true    /*!< DMA1 Stream 2 global Interrupt                                    */
#define DMA1_Stream3_IRQn_ENABLE false    /*!< DMA1 Stream 3 global Interrupt                                    */
#define DMA1_Stream4_IRQn_ENABLE true    /*!< DMA1 Stream 4 global Interrupt                                    */
#define DMA1_Stream5_IRQn_ENABLE false    /*!< DMA1 Stream 5 global Interrupt                                    */
#define DMA1_Stream6_IRQn_ENABLE false    /*!< DMA1 Stream 6 global Interrupt                                    */
#define ADC_IRQn_ENABLE true    /*!< ADC1, ADC2 and ADC3 global Interrupts                             */
/******  STM32 F40x and F41x specific Interrupt Numbers ********************************************************/
#define CAN1_TX_IRQn_ENABLE false    /*!< CAN1 TX Interrupt                                                 */
#define CAN1_RX0_IRQn_ENABLE false    /*!< CAN1 RX0 Interrupt                                                */
#define CAN1_RX1_IRQn_ENABLE false    /*!< CAN1 RX1 Interrupt                                                */
#define CAN1_SCE_IRQn_ENABLE false    /*!< CAN1 SCE Interrupt                                                */
#define EXTI9_5_IRQn_ENABLE false    /*!< External Line[9:5] Interrupts                                     */
#define TIM1_BRK_TIM9_IRQn_ENABLE false    /*!< TIM1 Break interrupt and TIM9 global interrupt                    */
#define TIM1_UP_TIM10_IRQn_ENABLE false    /*!< TIM1 Update Interrupt and TIM10 global interrupt                  */
#define TIM1_TRG_COM_TIM11_IRQn_ENABLE false    /*!< TIM1 Trigger and Commutation Interrupt and TIM11 global interrupt */
#define TIM1_CC_IRQn_ENABLE false    /*!< TIM1 Capture Compare Interrupt                                    */
#define TIM2_IRQn_ENABLE false    /*!< TIM2 global Interrupt                                             */
#define TIM3_IRQn_ENABLE false    /*!< TIM3 global Interrupt                                             */
#define TIM4_IRQn_ENABLE false    /*!< TIM4 global Interrupt                                             */
#define I2C1_EV_IRQn_ENABLE false    /*!< I2C1 Event Interrupt                                              */
#define I2C1_ER_IRQn_ENABLE false    /*!< I2C1 Error Interrupt                                              */
#define I2C2_EV_IRQn_ENABLE false    /*!< I2C2 Event Interrupt                                              */
#define I2C2_ER_IRQn_ENABLE false    /*!< I2C2 Error Interrupt                                              */
#define SPI1_IRQn_ENABLE false    /*!< SPI1 global Interrupt                                             */
#define SPI2_IRQn_ENABLE false    /*!< SPI2 global Interrupt                                             */
#define USART1_IRQn_ENABLE false    /*!< USART1 global Interrupt                                           */
#define USART2_IRQn_ENABLE false    /*!< USART2 global Interrupt                                           */
#define USART3_IRQn_ENABLE true    /*!< USART3 global Interrupt                                           */
#define EXTI15_10_IRQn_ENABLE false    /*!< External Line[15:10] Interrupts                                   */
#define RTC_Alarm_IRQn_ENABLE false    /*!< RTC Alarm (A and B) through EXTI Line Interrupt                   */
#define OTG_FS_WKUP_IRQn_ENABLE false    /*!< USB OTG FS Wakeup through EXTI line interrupt                     */
#define TIM8_BRK_TIM12_IRQn_ENABLE false    /*!< TIM8 Break Interrupt and TIM12 global interrupt                   */
#define TIM8_UP_TIM13_IRQn_ENABLE false    /*!< TIM8 Update Interrupt and TIM13 global interrupt                  */
#define TIM8_TRG_COM_TIM14_IRQn_ENABLE false    /*!< TIM8 Trigger and Commutation Interrupt and TIM14 global interrupt */
#define TIM8_CC_IRQn_ENABLE false    /*!< TIM8 Capture Compare Interrupt                                    */
#define DMA1_Stream7_IRQn_ENABLE false    /*!< DMA1 Stream7 Interrupt                                            */
#define FSMC_IRQn_ENABLE false    /*!< FSMC global Interrupt                                             */
#define SDIO_IRQn_ENABLE false    /*!< SDIO global Interrupt                                             */
#define TIM5_IRQn_ENABLE false    /*!< TIM5 global Interrupt                                             */
#define SPI3_IRQn_ENABLE false    /*!< SPI3 global Interrupt                                             */
#define UART4_IRQn_ENABLE true    /*!< UART4 global Interrupt                                            */
#define UART5_IRQn_ENABLE false    /*!< UART5 global Interrupt                                            */
#define TIM6_DAC_IRQn_ENABLE false    /*!< TIM6 global and DAC1&2 underrun error  interrupts                 */
#define TIM7_IRQn_ENABLE false    /*!< TIM7 global interrupt                                             */
#define DMA2_Stream0_IRQn_ENABLE true    /*!< DMA2 Stream 0 global Interrupt                                    */
#define DMA2_Stream1_IRQn_ENABLE false    /*!< DMA2 Stream 1 global Interrupt                                    */
#define DMA2_Stream2_IRQn_ENABLE false    /*!< DMA2 Stream 2 global Interrupt                                    */
#define DMA2_Stream3_IRQn_ENABLE false    /*!< DMA2 Stream 3 global Interrupt                                    */
#define DMA2_Stream4_IRQn_ENABLE false    /*!< DMA2 Stream 4 global Interrupt                                    */
#define ETH_IRQn_ENABLE false    /*!< Ethernet global Interrupt                                         */
#define ETH_WKUP_IRQn_ENABLE false    /*!< Ethernet Wakeup through EXTI line Interrupt                       */
#define CAN2_TX_IRQn_ENABLE false    /*!< CAN2 TX Interrupt                                                 */
#define CAN2_RX0_IRQn_ENABLE false    /*!< CAN2 RX0 Interrupt                                                */
#define CAN2_RX1_IRQn_ENABLE false    /*!< CAN2 RX1 Interrupt                                                */
#define CAN2_SCE_IRQn_ENABLE false    /*!< CAN2 SCE Interrupt                                                */
#define OTG_FS_IRQn_ENABLE false    /*!< USB OTG FS global Interrupt                                       */
#define DMA2_Stream5_IRQn_ENABLE false    /*!< DMA2 Stream 5 global interrupt                                    */
#define DMA2_Stream6_IRQn_ENABLE false    /*!< DMA2 Stream 6 global interrupt                                    */
#define DMA2_Stream7_IRQn_ENABLE false    /*!< DMA2 Stream 7 global interrupt                                    */
#define USART6_IRQn_ENABLE false    /*!< USART6 global interrupt                                           */
#define I2C3_EV_IRQn_ENABLE false    /*!< I2C3 event interrupt                                              */
#define I2C3_ER_IRQn_ENABLE false    /*!< I2C3 error interrupt                                              */
#define OTG_HS_EP1_OUT_IRQn_ENABLE false    /*!< USB OTG HS End Point 1 Out global interrupt                       */
#define OTG_HS_EP1_IN_IRQn_ENABLE false    /*!< USB OTG HS End Point 1 In global interrupt                        */
#define OTG_HS_WKUP_IRQn_ENABLE false    /*!< USB OTG HS Wakeup through EXTI interrupt                          */
#define OTG_HS_IRQn_ENABLE false    /*!< USB OTG HS global interrupt                                       */
#define DCMI_IRQn_ENABLE false    /*!< DCMI global interrupt                                             */
#define CRYP_IRQn_ENABLE false    /*!< CRYP crypto global interrupt                                      */
#define HASH_RNG_IRQn_ENABLE false    /*!< Hash and Rng global interrupt                                     */
#define FPU_IRQn_ENABLE false      /*!< FPU global interrupt                                              */

/**
 * Macro to check, whether the ENABLE define for the selected interrupt is set.
 * If it were unset, the instruction pointer will jump to an arbitrary memory location,
 * with no ISR defined.
 */
#define CHECK_INTERRUPT_HANDER_ENABLED(int_num) ( \
                                                 /******  Cortex-M4 Processor Exceptions Numbers ****************************************************************/ \
                                                 IRQn::NonMaskableInt_IRQn == int_num ? NonMaskableInt_IRQn_ENABLE : \
                                                 IRQn::MemoryManagement_IRQn == \
                                                 int_num ? MemoryManagement_IRQn_ENABLE : \
                                                 IRQn::BusFault_IRQn == int_num ? BusFault_IRQn_ENABLE : \
                                                 IRQn::UsageFault_IRQn == int_num ? UsageFault_IRQn_ENABLE : \
                                                 IRQn::SVCall_IRQn == int_num ? SVCall_IRQn_ENABLE : \
                                                 IRQn::DebugMonitor_IRQn == int_num ? DebugMonitor_IRQn_ENABLE : \
                                                 IRQn::PendSV_IRQn == int_num ? PendSV_IRQn_ENABLE : \
                                                 IRQn::SysTick_IRQn == int_num ? SysTick_IRQn_ENABLE : \
                                                 /******  STM32 specific Interrupt Numbers **********************************************************************/ \
                                                 IRQn::WWDG_IRQn == int_num ? WWDG_IRQn_ENABLE : \
                                                 IRQn::PVD_IRQn == int_num ? PVD_IRQn_ENABLE : \
                                                 IRQn::TAMP_STAMP_IRQn == int_num ? TAMP_STAMP_IRQn_ENABLE : \
                                                 IRQn::RTC_WKUP_IRQn == int_num ? RTC_WKUP_IRQn_ENABLE : \
                                                 IRQn::FLASH_IRQn == int_num ? FLASH_IRQn_ENABLE : \
                                                 IRQn::RCC_IRQn == int_num ? RCC_IRQn_ENABLE : \
                                                 IRQn::EXTI0_IRQn == int_num ? EXTI0_IRQn_ENABLE : \
                                                 IRQn::EXTI1_IRQn == int_num ? EXTI1_IRQn_ENABLE : \
                                                 IRQn::EXTI2_IRQn == int_num ? EXTI2_IRQn_ENABLE : \
                                                 IRQn::EXTI3_IRQn == int_num ? EXTI3_IRQn_ENABLE : \
                                                 IRQn::EXTI4_IRQn == int_num ? EXTI4_IRQn_ENABLE : \
                                                 IRQn::DMA1_Stream0_IRQn == int_num ? DMA1_Stream0_IRQn_ENABLE : \
                                                 IRQn::DMA1_Stream1_IRQn == int_num ? DMA1_Stream1_IRQn_ENABLE : \
                                                 IRQn::DMA1_Stream2_IRQn == int_num ? DMA1_Stream2_IRQn_ENABLE : \
                                                 IRQn::DMA1_Stream3_IRQn == int_num ? DMA1_Stream3_IRQn_ENABLE : \
                                                 IRQn::DMA1_Stream4_IRQn == int_num ? DMA1_Stream4_IRQn_ENABLE : \
                                                 IRQn::DMA1_Stream5_IRQn == int_num ? DMA1_Stream5_IRQn_ENABLE : \
                                                 IRQn::DMA1_Stream6_IRQn == int_num ? DMA1_Stream6_IRQn_ENABLE : \
                                                 IRQn::ADC_IRQn == int_num ? ADC_IRQn_ENABLE : \
                                                 IRQn::CAN1_TX_IRQn == int_num ? CAN1_TX_IRQn_ENABLE : \
                                                 IRQn::CAN1_RX0_IRQn == int_num ? CAN1_RX0_IRQn_ENABLE : \
                                                 IRQn::CAN1_RX1_IRQn == int_num ? CAN1_RX1_IRQn_ENABLE : \
                                                 IRQn::CAN1_SCE_IRQn == int_num ? CAN1_SCE_IRQn_ENABLE : \
                                                 IRQn::EXTI9_5_IRQn == int_num ? EXTI9_5_IRQn_ENABLE : \
                                                 IRQn::TIM1_BRK_TIM9_IRQn == int_num ? TIM1_BRK_TIM9_IRQn_ENABLE : \
                                                 IRQn::TIM1_UP_TIM10_IRQn == int_num ? TIM1_UP_TIM10_IRQn_ENABLE : \
                                                 IRQn::TIM1_TRG_COM_TIM11_IRQn == \
                                                 int_num ? TIM1_TRG_COM_TIM11_IRQn_ENABLE : \
                                                 IRQn::TIM1_CC_IRQn == int_num ? TIM1_CC_IRQn_ENABLE : \
                                                 IRQn::TIM2_IRQn == int_num ? TIM2_IRQn_ENABLE : \
                                                 IRQn::TIM3_IRQn == int_num ? TIM3_IRQn_ENABLE : \
                                                 IRQn::TIM4_IRQn == int_num ? TIM4_IRQn_ENABLE : \
                                                 IRQn::I2C1_EV_IRQn == int_num ? I2C1_EV_IRQn_ENABLE : \
                                                 IRQn::I2C1_ER_IRQn == int_num ? I2C1_ER_IRQn_ENABLE : \
                                                 IRQn::I2C2_EV_IRQn == int_num ? I2C2_EV_IRQn_ENABLE : \
                                                 IRQn::I2C2_ER_IRQn == int_num ? I2C2_ER_IRQn_ENABLE : \
                                                 IRQn::SPI1_IRQn == int_num ? SPI1_IRQn_ENABLE : \
                                                 IRQn::SPI2_IRQn == int_num ? SPI2_IRQn_ENABLE : \
                                                 IRQn::USART1_IRQn == int_num ? USART1_IRQn_ENABLE : \
                                                 IRQn::USART2_IRQn == int_num ? USART2_IRQn_ENABLE : \
                                                 IRQn::USART3_IRQn == int_num ? USART3_IRQn_ENABLE : \
                                                 IRQn::EXTI15_10_IRQn == int_num ? EXTI15_10_IRQn_ENABLE : \
                                                 IRQn::RTC_Alarm_IRQn == int_num ? RTC_Alarm_IRQn_ENABLE : \
                                                 IRQn::OTG_FS_WKUP_IRQn == int_num ? OTG_FS_WKUP_IRQn_ENABLE : \
                                                 IRQn::TIM8_BRK_TIM12_IRQn == int_num ? TIM8_BRK_TIM12_IRQn_ENABLE : \
                                                 IRQn::TIM8_UP_TIM13_IRQn == int_num ? TIM8_UP_TIM13_IRQn_ENABLE : \
                                                 IRQn::TIM8_TRG_COM_TIM14_IRQn == \
                                                 int_num ? TIM8_TRG_COM_TIM14_IRQn_ENABLE : \
                                                 IRQn::TIM8_CC_IRQn == int_num ? TIM8_CC_IRQn_ENABLE : \
                                                 IRQn::DMA1_Stream7_IRQn == int_num ? DMA1_Stream7_IRQn_ENABLE : \
                                                 IRQn::FSMC_IRQn == int_num ? FSMC_IRQn_ENABLE : \
                                                 IRQn::SDIO_IRQn == int_num ? SDIO_IRQn_ENABLE : \
                                                 IRQn::TIM5_IRQn == int_num ? TIM5_IRQn_ENABLE : \
                                                 IRQn::SPI3_IRQn == int_num ? SPI3_IRQn_ENABLE : \
                                                 IRQn::UART4_IRQn == int_num ? UART4_IRQn_ENABLE : \
                                                 IRQn::UART5_IRQn == int_num ? UART5_IRQn_ENABLE : \
                                                 IRQn::TIM6_DAC_IRQn == int_num ? TIM6_DAC_IRQn_ENABLE : \
                                                 IRQn::TIM7_IRQn == int_num ? TIM7_IRQn_ENABLE : \
                                                 IRQn::DMA2_Stream0_IRQn == int_num ? DMA2_Stream0_IRQn_ENABLE : \
                                                 IRQn::DMA2_Stream1_IRQn == int_num ? DMA2_Stream1_IRQn_ENABLE : \
                                                 IRQn::DMA2_Stream2_IRQn == int_num ? DMA2_Stream2_IRQn_ENABLE : \
                                                 IRQn::DMA2_Stream3_IRQn == int_num ? DMA2_Stream3_IRQn_ENABLE : \
                                                 IRQn::DMA2_Stream4_IRQn == int_num ? DMA2_Stream4_IRQn_ENABLE : \
                                                 IRQn::ETH_IRQn == int_num ? ETH_IRQn_ENABLE : \
                                                 IRQn::ETH_WKUP_IRQn == int_num ? ETH_WKUP_IRQn_ENABLE : \
                                                 IRQn::CAN2_TX_IRQn == int_num ? CAN2_TX_IRQn_ENABLE : \
                                                 IRQn::CAN2_RX0_IRQn == int_num ? CAN2_RX0_IRQn_ENABLE : \
                                                 IRQn::CAN2_RX1_IRQn == int_num ? CAN2_RX1_IRQn_ENABLE : \
                                                 IRQn::CAN2_SCE_IRQn == int_num ? CAN2_SCE_IRQn_ENABLE : \
                                                 IRQn::OTG_FS_IRQn == int_num ? OTG_FS_IRQn_ENABLE : \
                                                 IRQn::DMA2_Stream5_IRQn == int_num ? DMA2_Stream5_IRQn_ENABLE : \
                                                 IRQn::DMA2_Stream6_IRQn == int_num ? DMA2_Stream6_IRQn_ENABLE : \
                                                 IRQn::DMA2_Stream7_IRQn == int_num ? DMA2_Stream7_IRQn_ENABLE : \
                                                 IRQn::USART6_IRQn == int_num ? USART6_IRQn_ENABLE : \
                                                 IRQn::I2C3_EV_IRQn == int_num ? I2C3_EV_IRQn_ENABLE : \
                                                 IRQn::I2C3_ER_IRQn == int_num ? I2C3_ER_IRQn_ENABLE : \
                                                 IRQn::OTG_HS_EP1_OUT_IRQn == int_num ? OTG_HS_EP1_OUT_IRQn_ENABLE : \
                                                 IRQn::OTG_HS_EP1_IN_IRQn == int_num ? OTG_HS_EP1_IN_IRQn_ENABLE : \
                                                 IRQn::OTG_HS_WKUP_IRQn == int_num ? OTG_HS_WKUP_IRQn_ENABLE : \
                                                 IRQn::OTG_HS_IRQn == int_num ? OTG_HS_IRQn_ENABLE : \
                                                 IRQn::DCMI_IRQn == int_num ? DCMI_IRQn_ENABLE : \
                                                 IRQn::CRYP_IRQn == int_num ? CRYP_IRQn_ENABLE : \
                                                 IRQn::HASH_RNG_IRQn == int_num ? HASH_RNG_IRQn_ENABLE : \
                                                 IRQn::FPU_IRQn == int_num ? FPU_IRQn_ENABLE : \
                                                 false)

#endif /* PROJECT_CONFIG_NVIC_CONFIG_H_ */
