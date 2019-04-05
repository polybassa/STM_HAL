/*
 * NVIC.cpp
 *
 *  Created on: Nov 9, 2018
 *      Author: Henning Mende
 *
 *  Abstraction for the nested vector interrupt controller.
 */

#include "Nvic.h"
#include "SEGGER_SYSVIEW.h"

namespace hal
{
Nvic::CallbackArray Nvic::NvicCallbacks;
Nvic::GetInterruptStatusProcedureArray Nvic::NvicGetInterruptStatusProcedeures;
Nvic::ClearInterruptProcedureArray Nvic::NvicClearInterruptProcedeures;

void Nvic::setPriority(const uint16_t& prio) const
{
    NVIC_SetPriority(mInterruptChannel, prio);
}

bool Nvic::getStatus(void) const
{
    if (NvicGetInterruptStatusProcedeures[mDescription]) {
        return NvicGetInterruptStatusProcedeures[mDescription]();
    }

    return false;
}

void Nvic::clearInterruptBit(void) const
{
    if (NvicClearInterruptProcedeures[mDescription]) {
        NvicClearInterruptProcedeures[mDescription]();
    }
}

void Nvic::executeCallback(void) const
{
    if (NvicCallbacks[mDescription]) {
        NvicCallbacks[mDescription]();
    }
}

void Nvic::handleInterrupt(void) const
{
    if (getStatus()) {
        executeCallback();
        clearInterruptBit();
    }
}

void Nvic::registerGetInterruptStatusProcedure(std::function<bool(void)> f) const
{
    NvicGetInterruptStatusProcedeures[mDescription] = f;
}

void Nvic::registerClearInterruptProcedure(std::function<void(void)> f) const
{
    NvicClearInterruptProcedeures[mDescription] = f;
}

void Nvic::registerInterruptCallback(std::function<void(void)> f) const
{
    NvicCallbacks[mDescription] = f;
}

void Nvic::unregisterInterruptCallback(void) const
{
    NvicCallbacks[mDescription] = nullptr;
}

bool Nvic::enable(void) const
{
    if (NvicGetInterruptStatusProcedeures[mDescription] &&
        NvicClearInterruptProcedeures[mDescription])
    {
//		NVIC_InitTypeDef NVIC_InitStructure = mConfiguration;
//		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//		NVIC_Init(&NVIC_InitStructure);
//		NVIC_EnableIRQ(static_cast<const enum IRQn>(mConfiguration.NVIC_IRQChannel));
        NVIC_EnableIRQ(mInterruptChannel);
        return true;
    } else {
        return false;
    }
}

void Nvic::disable(void) const
{
//	NVIC_InitTypeDef NVIC_InitStructure = mConfiguration;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
//	NVIC_Init(&NVIC_InitStructure);
//	NVIC_DisableIRQ(static_cast<const enum IRQn>(mConfiguration.NVIC_IRQChannel));
    NVIC_DisableIRQ(mInterruptChannel);
}

constexpr const std::array<const Nvic, Nvic::Description::__ENUM__SIZE + 1> Factory<Nvic>::Container;
}

// ****** Interrupt service routines (mapped to the configured hal::Nvics) *********************************
extern "C" {
/******  Cortex-M4 Processor Exceptions Numbers ****************************************************************/
#if NonMaskableInt_IRQn_ENABLE
void NonMaskableInt_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<NonMaskableInt_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if MemoryManagement_IRQn_ENABLE
void MemoryManagement_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<MemoryManagement_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if BusFault_IRQn_ENABLE
void BusFault_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<BusFault_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if UsageFault_IRQn_ENABLE
void UsageFault_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<UsageFault_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if SVCall_IRQn_ENABLE
void SVCall_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<SVCall_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if DebugMonitor_IRQn_ENABLE
void DebugMonitor_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<DebugMonitor_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if PendSV_IRQn_ENABLE
void PendSV_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<PendSV_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if SysTick_IRQn_ENABLE
void SysTick_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<SysTick_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

/******  STM32 specific Interrupt Numbers **********************************************************************/
#if WWDG_IRQn_ENABLE
void WWDG_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<WWDG_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if PVD_IRQn_ENABLE
void PVD_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<PVD_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if TAMP_STAMP_IRQn_ENABLE
void TAMP_STAMP_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<TAMP_STAMP_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if RTC_WKUP_IRQn_ENABLE
void RTC_WKUP_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<RTC_WKUP_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if FLASH_IRQn_ENABLE
void FLASH_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<FLASH_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if RCC_IRQn_ENABLE
void RCC_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<RCC_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if EXTI0_IRQn_ENABLE
void EXTI0_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<EXTI0_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if EXTI1_IRQn_ENABLE
void EXTI1_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<EXTI1_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if EXTI2_IRQn_ENABLE
void EXTI2_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<EXTI2_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if EXTI3_IRQn_ENABLE
void EXTI3_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<EXTI3_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if EXTI4_IRQn_ENABLE
void EXTI4_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<EXTI4_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if DMA1_Stream0_IRQn_ENABLE
void DMA1_Stream0_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<DMA1_Stream0_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if DMA1_Stream1_IRQn_ENABLE
void DMA1_Stream1_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<DMA1_Stream1_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if DMA1_Stream2_IRQn_ENABLE
void DMA1_Stream2_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<DMA1_Stream2_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if DMA1_Stream3_IRQn_ENABLE
void DMA1_Stream3_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<DMA1_Stream3_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if DMA1_Stream4_IRQn_ENABLE
void DMA1_Stream4_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<DMA1_Stream4_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if DMA1_Stream5_IRQn_ENABLE
void DMA1_Stream5_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<DMA1_Stream5_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if DMA1_Stream6_IRQn_ENABLE
void DMA1_Stream6_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<DMA1_Stream6_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if ADC_IRQn_ENABLE
void ADC_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<ADC_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

/******  STM32 F40x and F41x specific Interrupt Numbers ********************************************************/
#if CAN1_TX_IRQn_ENABLE
void CAN1_TX_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<CAN1_TX_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if CAN1_RX0_IRQn_ENABLE
void CAN1_RX0_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<CAN1_RX0_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if CAN1_RX1_IRQn_ENABLE
void CAN1_RX1_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<CAN1_RX1_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if CAN1_SCE_IRQn_ENABLE
void CAN1_SCE_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<CAN1_SCE_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if EXTI9_5_IRQn_ENABLE
void EXTI9_5_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<EXTI9_5_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if TIM1_BRK_TIM9_IRQn_ENABLE
void TIM1_BRK_TIM9_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<TIM1_BRK_TIM9_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if TIM1_UP_TIM10_IRQn_ENABLE
void TIM1_UP_TIM10_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<TIM1_UP_TIM10_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if TIM1_TRG_COM_TIM11_IRQn_ENABLE
void TIM1_TRG_COM_TIM11_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<TIM1_TRG_COM_TIM11_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if TIM1_CC_IRQn_ENABLE
void TIM1_CC_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<TIM1_CC_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if TIM2_IRQn_ENABLE
void TIM2_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<TIM2_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if TIM3_IRQn_ENABLE
void TIM3_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<TIM3_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if TIM4_IRQn_ENABLE
void TIM4_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<TIM4_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if I2C1_EV_IRQn_ENABLE
void I2C1_EV_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<I2C1_EV_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if I2C1_ER_IRQn_ENABLE
void I2C1_ER_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<I2C1_ER_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if I2C2_EV_IRQn_ENABLE
void I2C2_EV_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<I2C2_EV_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if I2C2_ER_IRQn_ENABLE
void I2C2_ER_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<I2C2_ER_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if SPI1_IRQn_ENABLE
void SPI1_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<SPI1_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if SPI2_IRQn_ENABLE
void SPI2_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<SPI2_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if USART1_IRQn_ENABLE
void USART1_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<USART1_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if USART2_IRQn_ENABLE
void USART2_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<USART2_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if USART3_IRQn_ENABLE
void USART3_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<USART3_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if EXTI15_10_IRQn_ENABLE
void EXTI15_10_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<EXTI15_10_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if RTC_Alarm_IRQn_ENABLE
void RTC_Alarm_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<RTC_Alarm_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if OTG_FS_WKUP_IRQn_ENABLE
void OTG_FS_WKUP_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<OTG_FS_WKUP_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if TIM8_BRK_TIM12_IRQn_ENABLE
void TIM8_BRK_TIM12_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<TIM8_BRK_TIM12_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if TIM8_UP_TIM13_IRQn_ENABLE
void TIM8_UP_TIM13_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<TIM8_UP_TIM13_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if TIM8_TRG_COM_TIM14_IRQn_ENABLE
void TIM8_TRG_COM_TIM14_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<TIM8_TRG_COM_TIM14_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if TIM8_CC_IRQn_ENABLE
void TIM8_CC_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<TIM8_CC_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if DMA1_Stream7_IRQn_ENABLE
void DMA1_Stream7_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<DMA1_Stream7_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if FSMC_IRQn_ENABLE
void FSMC_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<FSMC_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if SDIO_IRQn_ENABLE
void SDIO_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<SDIO_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if TIM5_IRQn_ENABLE
void TIM5_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<TIM5_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if SPI3_IRQn_ENABLE
void SPI3_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<SPI3_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if UART4_IRQn_ENABLE
void UART4_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<UART4_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if UART5_IRQn_ENABLE
void UART5_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<UART5_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if TIM6_DAC_IRQn_ENABLE
void TIM6_DAC_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<TIM6_DAC_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if TIM7_IRQn_ENABLE
void TIM7_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<TIM7_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if DMA2_Stream0_IRQn_ENABLE
void DMA2_Stream0_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<DMA2_Stream0_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if DMA2_Stream1_IRQn_ENABLE
void DMA2_Stream1_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<DMA2_Stream1_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if DMA2_Stream2_IRQn_ENABLE
void DMA2_Stream2_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<DMA2_Stream2_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if DMA2_Stream3_IRQn_ENABLE
void DMA2_Stream3_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<DMA2_Stream3_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if DMA2_Stream4_IRQn_ENABLE
void DMA2_Stream4_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<DMA2_Stream4_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if ETH_IRQn_ENABLE
void ETH_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<ETH_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if ETH_WKUP_IRQn_ENABLE
void ETH_WKUP_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<ETH_WKUP_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if CAN2_TX_IRQn_ENABLE
void CAN2_TX_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<CAN2_TX_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if CAN2_RX0_IRQn_ENABLE
void CAN2_RX0_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<CAN2_RX0_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if CAN2_RX1_IRQn_ENABLE
void CAN2_RX1_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<CAN2_RX1_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if CAN2_SCE_IRQn_ENABLE
void CAN2_SCE_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<CAN2_SCE_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if OTG_FS_IRQn_ENABLE
void OTG_FS_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<OTG_FS_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if DMA2_Stream5_IRQn_ENABLE
void DMA2_Stream5_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<DMA2_Stream5_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if DMA2_Stream6_IRQn_ENABLE
void DMA2_Stream6_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<DMA2_Stream6_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if DMA2_Stream7_IRQn_ENABLE
void DMA2_Stream7_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<DMA2_Stream7_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if USART6_IRQn_ENABLE
void USART6_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<USART6_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if I2C3_EV_IRQn_ENABLE
void I2C3_EV_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<I2C3_EV_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if I2C3_ER_IRQn_ENABLE
void I2C3_ER_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<I2C3_ER_IRQn>();
    nvic.handleInterrupt();
}
#endif

#if OTG_HS_EP1_OUT_IRQn_ENABLE
void OTG_HS_EP1_OUT_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<OTG_HS_EP1_OUT_IRQn>();
    nvic.handleInterrupt();
}
#endif

#if OTG_HS_EP1_IN_IRQn_ENABLE
void OTG_HS_EP1_IN_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<OTG_HS_EP1_IN_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if OTG_HS_WKUP_IRQn_ENABLE
void OTG_HS_WKUP_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<OTG_HS_WKUP_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if OTG_HS_IRQn_ENABLE
void OTG_HS_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<OTG_HS_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if DCMI_IRQn_ENABLE
void DCMI_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<DCMI_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if CRYP_IRQn_ENABLE
void CRYP_IRQHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<CRYP_IRQn>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if HASH_RNG_IRQn_ENABLE_ENABLE
void HASH_RNG_IRQn_ENABLHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<HASH_RNG_IRQn_ENABLE>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif

#if FPU_IRQn_ENABLE_ENABLE
void FPU_IRQn_ENABLHandler(void)
{
    SEGGER_SYSVIEW_RecordEnterISR();
    constexpr const hal::Nvic& nvic = hal::Factory<hal::Nvic>::getByIrqChannel<FPU_IRQn_ENABLE>();
    nvic.handleInterrupt();
    SEGGER_SYSVIEW_RecordExitISR();
}
#endif
}
