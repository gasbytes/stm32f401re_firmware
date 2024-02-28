#ifndef USART_PRINTF_H
#define USART_PRINTF_H

#include "../../inc/peripherals.h"

/**
 * @brief Struct Pointer for RCC Peripherals assigned with fixed address specified in reference manual.
 *
 * See Memory map, Section 2.3.
 **/
extern RCC_t * const RCC;

/**
 * @brief Struct Pointer for GPIOA Peripherals assigned with fixed address specified in reference manual.
 *
 * See Memory map, Section 2.3.
 **/
extern GPIOx_t * const GPIOA;

/*
 * @brieft Struct pointer for the UART2 Peripherals assigned with fixed address specified in reference manual.
 *
 * See Memory Map, Section 2.3.
 * */
extern USART_t * const USART2;

/*
 * @brief Struct Pointer for SYST (System Timer) assigned with fixed address specified in the datasheet.
 *
 * See section 4.4 System timer, SysTick (ARM-cortex-m4 datasheet).
 * */
extern SYST_t * const SYST;

#define PA2 2

#define CPU_FREQUENCY 16000000

#endif // !USART_PRINTF_H

