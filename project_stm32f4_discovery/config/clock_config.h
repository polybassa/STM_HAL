/*
 * clock_config.h
 *
 *  Created on: Sep 20, 2018
 *      Author: Henning Mende
 *
 *  Configuration of the System clock. This clock generates the SystemTick, which drives the operating system.
 *  If this confifguration doesn't match your hardware configuration, your whole RTOS will run in the wrong speed.
 */

#ifndef PROJECT_CLOCK_CONFIG_H_
#define PROJECT_CLOCK_CONFIG_H_

#define EXT_CLOCK_VAL_HZ         8000000     /*!< Value of the External oscillator in Hz */

/* test whether HSE_VALUE is set to full MHz values. */
#define MEGAHERTZ    1000000
#define PLL_M_CHECK (EXT_CLOCK_VAL_HZ / MEGAHERTZ)
#if PLL_M_CHECK* MEGAHERTZ != EXT_CLOCK_VAL_HZ
#error "HSE_VALUE not given in full MHz thus the calculation of PLL_M in system_stm32f4xx.c has to be adapted!"
#endif

#define HSE_VALUE    ((uint32_t)EXT_CLOCK_VAL_HZ) /*!< casted external oscillator frequency for the std periph lib */

#undef MEGAHERTZ
#undef PLL_M_CHECK
#endif /* PROJECT_CLOCK_CONFIG_H_ */
