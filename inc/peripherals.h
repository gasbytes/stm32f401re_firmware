/**
 *@brief Peripheral initialization using structs.
 **/
#ifndef PERIPHERLAS_H
#define PERIPHERLAS_H

#include <stdint.h>

/*
 * __IO is a macro for volatile - defined in STM32 standard peripheral library header files
 * */
#define __IO volatile

/*
 * Simple struct that holds the
 * names of the GPIO (General Purpose Input Output) registers.
 * Each field of the struct is long 32 bits, since 
 * yes, yup, you guessed it, each register is 32 bits long.
 * */
typedef struct GPIOx_t{
	__IO uint32_t GPIOx_MODER; 		
	__IO uint32_t GPIOx_OTYPER;	
	__IO uint32_t GPIOx_OSPEEDER;	
	__IO uint32_t GPIOx_PUPDR;	
	__IO uint32_t GPIOx_IDR;
	__IO uint32_t GPIOx_ODR;	
	__IO uint32_t GPIOx_BSRR;	
	__IO uint32_t GPIOx_LCKR;
	__IO uint32_t GPIOx_AFRL;
	__IO uint32_t GPIOx_AFRH;
}GPIOx_t;

/*
 * Simple struct that holds the
 * names of the RCC (Reset and Clock Control unit) registers.
 * Each field of the struct is long 32 bits, since 
 * yes, yup, you guessed it, each register is 32 bits long.
 * */
typedef struct RCC_t{
	__IO uint32_t RCC_CR;
	__IO uint32_t RCC_PLLCFGT;
	__IO uint32_t RCC_CFGR;
	__IO uint32_t RCC_CIR;
	__IO uint32_t RCC_AHB1RSTR;
	__IO uint32_t RCC_AHB2RSTR;
	__IO uint32_t res1[2];
	__IO uint32_t RCC_APB1RSTR;
	__IO uint32_t RCC_APB2RSTR;
	__IO uint32_t res2[2];
	__IO uint32_t RCC_AHB1ENR;
	__IO uint32_t RCC_AHB2ENR;
	__IO uint32_t res3[2];
	__IO uint32_t RCC_APB1ENR;
	__IO uint32_t RCC_APB2ENR;
	__IO uint32_t res4[2];
	__IO uint32_t RCC_AHB1LPENR;
	__IO uint32_t RCC_AHB2LPENR;
	__IO uint32_t res5[2];
	__IO uint32_t RCC_APB1LPENR;
	__IO uint32_t RCC_APB2LPENR;
	__IO uint32_t res6[2];
	__IO uint32_t RCC_BDCR;
	__IO uint32_t RCC_CSR;
	__IO uint32_t res7[2];
	__IO uint32_t RCC_SSCGR;
	__IO uint32_t RCC_PLLI2SCFGR;
	__IO uint32_t res8;
	__IO uint32_t RCC_DCKCFGR;
}RCC_t;

/*
 * Simple struct that holds the names of 
 * the SysTick registers.
 * SysTick is a 24 bit system timer, that counts
 * down from the reload value (SYST_CVR) to zero, reloads (with SYST_RVR), that is 
 * wraps to, the value in the SYST_RVR register on the next clock edge
 * then counts down on subsequent clocks.
 *
 * In general: SYST_CVR == 0 ? generate interrupt, that is index x on the 
 * vector table.
 *
 * It's made out of 4 registers:
 *
 * SYST_CSR    -> control and status, used to enable/disable systick
 * SYST_RVR   -> reload value, an initial counter value
 * SYST_CVR   -> current value, a current counter value, decremented on each clock cycle
 * SYST_CALIB -> calibgration value, calibration register
 *
 * Section 4.4 of the arm-cortext-m4 datasheet.
 *
 * */
typedef struct SYST_t {
	__IO uint32_t SYST_CSR;
	__IO uint32_t SYST_RVR;
	__IO uint32_t SYST_CVR;
	__IO uint32_t SYST_CALIB;
} SYST_t;

/*
 * Simple struct that holds the names of the USART 
 * registers.
 * UART is not a protocol, it's actually an hardware device
 * that given as input parallel data, it outputs it as serial
 * bit by bit.
 *
 * Made of two lines: RX (receive) and TX (transmit).
 *
 * Universal because it can be configured to implement several serial
 * protocols.
 *
 * Asynchronous because it doesn't have a dedicated clock.
 *
 * Two devices to be able to communicate with each other using UART, they 
 * must have the same baud rate (actually 5% to 10% similar to each other).
 * They must have the same standard, the line driver, usually the de-facto standard is
 * RS232.
 * The line driver basically takes logic i/o from the uart and transform it into
 * electrical voltages (and viceversa).
 * */
typedef struct USART_t {
	__IO uint32_t USART_SR;
	__IO uint32_t USART_DR;
	__IO uint32_t USART_BRR;
	__IO uint32_t USART_CR1;
	__IO uint32_t USART_CR2;
	__IO uint32_t USART_CR3;
	__IO uint32_t USART_GTPR;
} USART_t;

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

#endif
