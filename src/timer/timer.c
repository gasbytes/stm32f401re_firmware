/*
 *@brief simple redirect printf to uart project
 **/
#include "../../inc/peripherals.h"
#include <stddef.h>
#include "timer.h"
#include <stdint.h>
#include <stdio.h>

#define PA2 2
#define pin5 5
#define MODER 2
#define CPU_FREQUENCY 16000000

/**
 * @brief Struct Pointer for RCC Peripherals assigned with fixed address specified in reference manual.
 *
 * See Memory map, Section 2.3.
 **/
RCC_t * const RCC = (RCC_t *) 0x40023800;

/**
 * @brief Struct Pointer for GPIOA Peripherals assigned with fixed address specified in reference manual.
 *
 * See Memory map, Section 2.3.
 **/
GPIOx_t * const GPIOA = (GPIOx_t *) 0x40020000;

/*
 * @brieft Struct pointer for the UART2 Peripherals assigned with fixed address specified in reference manual.
 *
 * See Memory Map, Section 2.3.
 * */
USART_t * const USART2 = (USART_t *) 0x40004400;

/*
 * @brief Struct Pointer for SYST (System Timer) assigned with fixed address specified in the datasheet.
 *
 * See section 4.4 System timer, SysTick (ARM-cortex-m4 datasheet).
 * */
SYST_t * const SYST = (SYST_t *) 0xE000E010;

/*
 * @brief Simple variable (and relative function) that keeps track of the number of ticks that happened since
 * the program started.
 *
 * Gets called everytime SysTick generates an interrupt.
 * */
static volatile uint32_t s_ticks;
void SysTick_Handler(void) {
    s_ticks++;
}

uint32_t get_systicks() {
    return s_ticks;
}

void setup_gpio(void) {
    // Enable Clock for the GPIOA Peripheral (Section 6.3.9)
    // This is a OR operation and lets us set individual bits,
    // in particular in this case we set the least significant bit to 1.
    // Since to enable the clock for the GPIOA we need to set 
    // bit 0 to 1 (0x00000001).
    //
    // This is needed so we can use pins PA2 and PA3.
    RCC->RCC_AHB1ENR |= 1;

    // Next step, is to configure (and then enable) the alternate function.
    // The alternate function is necessary so that we can 
    // use it for different tasks (instead of only analog, output etc...).
    //
    // From the document stm32f401re.pdf, we can find from the table 9
    // the alternate function mapping. In particular we can see that for the pins
    // PA2 (TX) and PA3 (RX) (the ones needed to UART through the st-link into my host system), we need to configure
    // the AF07.
    // First we clear the bits, then we set them.
    // In particular we need to set AF07 to 0111, which is...yes you guessed it
    // 7 in decimal.
    GPIOA->GPIOx_AFRL &= ~(0xF << (PA2 * 4)); // clear bits AFRL7[3:0]
    GPIOA->GPIOx_AFRL |= (7 << (PA2 * 4)); // set bits AFRL7[3:0]
    
    // Now we can enable the alternate function, by setting the moder
    // register to 10 (binary).
    //
    // Section 8.4.9.
    GPIOA->GPIOx_MODER &= ~(3 << (PA2 * 2)); // clear bits MODER1[1:0]
    GPIOA->GPIOx_MODER |= (2 << (PA2 * 2)); // set bits MODER1[1:0]
    
    // Reset MODER Bitfield of PA5 (Section 8.4.1)
    // This is a bitwise AND operation, and it lets us 
    // clear individual bits.
    // Explanation:
    // - Create a bitmask to clear configuration bits for the specified pin:
    //   - 3 (binary 11) is shifted left by (pin5 * MODER) bits.
    //   - ~(3 << (pin5 * MODER)) inverts all bits, creating a bitmask.
    // - Bitwise AND with the existing GPIOx_MODER to clear specific bits for the pin.
    //   - Leaves other bits unchanged.
    GPIOA->GPIOx_MODER &= ~(3 << (pin5 * MODER));

    // Write Value 1 to MODER Bitfield PA5 to configure PA5 as OUTPUT (Section 8.4.1)
    // This is a OR operation, in particular in this case we set to 1 the pin5 (01).
    GPIOA->GPIOx_MODER |=  (1 << (pin5 * MODER));
}

void setup_usart(void) {
    // We enable the clock for the USART2 interface,
    // in particular to do that we do the usual OR
    // operation, and we shift a 1 to the 17th position.
    // Section 6.3.11.
    RCC->RCC_APB1ENR |= (1 << 17);
    
    
    // Next step is to set the Baudrate, the rate at which information 
    // is transferred in a communication channel.
    // The usual standard is 9600, so maximum 9600 bits per second.
    // From the section 19.6.3 we can see that the MCU has a specific register for it,
    // the USART_BRR.
    // The value for this for the register, is 16MhZ (cpu's frequency) divided by the baud 
    // rate that we wanna communicate with.
    USART2->USART_BRR = CPU_FREQUENCY/9600;

    // We proceed then to enable the TX bit, that 
    // let's us actually transmit bits.
    //
    // Section 19.6.4.
    USART2->USART_CR1 |= (1 << 3); // CR1[3], transmitter enable
    USART2->USART_CR1 &= ~(1 << 12);
    
    // Finally, we can enable the USART2.
    //
    // Section 19.6.4
    //
    USART2->USART_CR1 |= (1 << 13); // CR1[13], USART enable.
}

void setup_systick(void) {
    // Enable Clock for SysTick (Section 6.3.12)
    RCC->RCC_APB2ENR |= (1 << 14);

    // We load the reload register with the maximum value (Section 4.4.2)
    SYST->SYST_RVR |= 0x00FFFFFF;

    // We set as internal source the processor clock, from where our systick will 'based' on, the 
    // 'rhythm' to derive from (Section 4.4.1).
    SYST->SYST_CSR |= (1 << 2);

    // We enable the callback feature, so that when the syst counter ends
    // it calls the handler that we set up. (Section 4.4.1.)
    SYST->SYST_CSR |= (1 << 1);

    // We set the initial value to zero.
    SYST->SYST_CVR |= 0;
    
    // We then proceed to finally enable the SysTick timer (Section 4.4.1)
    SYST->SYST_CSR |= (1 << 0);
}

void setup_timer(minimal_timer_t *timer, uint32_t wait_time, int auto_reset) {
    timer->wait_time = wait_time;
    timer->auto_reset = auto_reset;
    timer->target_time = get_systicks() + (wait_time);
}

int has_timer_elapsed(minimal_timer_t *timer) {
    uint32_t now = get_systicks();
    uint64_t delta;

    if (now >= timer->target_time) {
        delta = now - timer->target_time;

        timer->target_time = (now + timer->wait_time) - delta;

        return 1;
    }

    return 0;
}

void timer_reset(minimal_timer_t *timer) {
    setup_timer(timer, timer->wait_time, timer->auto_reset);
}

int main(void) {
    setup_gpio();
    setup_usart();
    setup_systick();

    minimal_timer_t timer;
    setup_timer(&timer, 5, 1);

    while(1) {
        if (has_timer_elapsed(&timer)) {
            GPIOA->GPIOx_ODR ^= (1 << pin5);
        }
    }

    return 0;
}
