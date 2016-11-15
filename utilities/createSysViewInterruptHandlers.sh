#!/bin/bash

ARRAY=( WWDG_IRQHandlerSV PVD_IRQHandlerSV TAMPER_STAMP_IRQHandlerSV RTC_WKUP_IRQHandlerSV FLASH_IRQHandlerSV RCC_IRQHandlerSV EXTI0_IRQHandlerSV EXTI1_IRQHandlerSV EXTI2_TS_IRQHandlerSV EXTI3_IRQHandlerSV EXTI4_IRQHandlerSV DMA1_Channel1_IRQHandlerSV DMA1_Channel2_IRQHandlerSV DMA1_Channel3_IRQHandlerSV DMA1_Channel4_IRQHandlerSV DMA1_Channel5_IRQHandlerSV DMA1_Channel6_IRQHandlerSV DMA1_Channel7_IRQHandlerSV ADC1_2_IRQHandlerSV USB_HP_CAN1_TX_IRQHandlerSV USB_LP_CAN1_RX0_IRQHandlerSV CAN1_RX1_IRQHandlerSV CAN1_SCE_IRQHandlerSV EXTI9_5_IRQHandlerSV TIM1_BRK_TIM15_IRQHandlerSV TIM1_UP_TIM16_IRQHandlerSV TIM1_TRG_COM_TIM17_IRQHandlerSV TIM1_CC_IRQHandlerSV TIM2_IRQHandlerSV TIM3_IRQHandlerSV TIM4_IRQHandlerSV I2C1_EV_IRQHandlerSV I2C1_ER_IRQHandlerSV I2C2_EV_IRQHandlerSV I2C2_ER_IRQHandlerSV SPI1_IRQHandlerSV SPI2_IRQHandlerSV USART1_IRQHandlerSV USART2_IRQHandlerSV USART3_IRQHandlerSV EXTI15_10_IRQHandlerSV RTC_Alarm_IRQHandlerSV USBWakeUp_IRQHandlerSV TIM8_BRK_IRQHandlerSV TIM8_UP_IRQHandlerSV TIM8_TRG_COM_IRQHandlerSV TIM8_CC_IRQHandlerSV ADC3_IRQHandlerSV SPI3_IRQHandlerSV UART4_IRQHandlerSV UART5_IRQHandlerSV TIM6_DAC_IRQHandlerSV TIM7_IRQHandlerSV DMA2_Channel1_IRQHandlerSV DMA2_Channel2_IRQHandlerSV DMA2_Channel3_IRQHandlerSV DMA2_Channel4_IRQHandlerSV DMA2_Channel5_IRQHandlerSV ADC4_IRQHandlerSV COMP1_2_3_IRQHandlerSV COMP4_5_6_IRQHandlerSV COMP7_IRQHandlerSV USB_HP_IRQHandlerSV USB_LP_IRQHandlerSV USBWakeUp_RMP_IRQHandlerSV FPU_IRQHandlerSV )

ARRAY_NO_WHITESPACE="$(echo -e "${ARRAY}" | tr -d '[[:space:]]')"

for handler in ${ARRAY[@]}
do
	handler="$(echo $handler | tr -d '[[:space:]]')"	
	echo -e '.section  .text.'$handler'\n.weak  '$handler'\n.type '$handler', %function\n'$handler':\n\tbl SEGGER_SYSVIEW_RecordEnterISR\n\tbl '${handler%??}'\n\tbl SEGGER_SYSVIEW_RecordExitISR\n.size  '$handler', .-'$handler'\n\n'
done
