#include "../../inc/peripherals.h"
#include "p_p.h"
#include <stddef.h>
#include <stdint.h>

#define PA2 2
#define PA3 3
#define CPU_FREQUENCY 16000000

/**
 * @brief Struct Pointer for RCC Peripherals assigned with fixed address specified in reference manual.
 *
 * See Memory map, Section 2.3.
 **/
RCC_t   * const RCC     = (RCC_t    *)  0x40023800; 

/**
 * @brief Struct Pointer for GPIOA Peripherals assigned with fixed address specified in reference manual.
 *
 * See Memory map, Section 2.3.
 **/
GPIOx_t * const GPIOA   = (GPIOx_t  *)  0x40020000;

/*
 * @brieft Struct pointer for the UART2 Peripherals assigned with fixed address specified in reference manual.
 *
 * See Memory Map, Section 2.3.
 * */
USART_t * const USART2 = (USART_t *)  0x40004400;

/*
 * @brief Struct Pointer for SYST (System Timer) assigned with fixed address specified in the datasheet.
 *
 * See section 4.4 System timer, SysTick (ARM-cortex-m4 datasheet).
 * */
SYST_t * const SYST = (SYST_t *) 0xE000E010;

void setup_gpio() {
    /** Enable CLOCK for GPIOA **/
    RCC->RCC_AHB1ENR |= 1;

    /** SET AF07 for PA2 **/
    GPIOA->GPIOx_AFRL &= ~(0xF << (2 * 4));
    GPIOA->GPIOx_AFRL |=  (7   << (2 * 4));

    /** SET AF07 for PA3 **/
    GPIOA->GPIOx_AFRL &= ~(0xF << (3 * 4));
    GPIOA->GPIOx_AFRL |=  (7   << (3 * 4));

    /** Set AF Mode for PA2 **/
    GPIOA->GPIOx_MODER &= ~(3 << (2 * 2));
    GPIOA->GPIOx_MODER |=  (2 << (2 * 2));

    /** Set AF Mode for PA3 **/
    GPIOA->GPIOx_MODER &= ~(3 << (3 * 2));
    GPIOA->GPIOx_MODER |=  (2 << (3 * 2));
}

void setup_usart() {
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
    USART2->USART_BRR &= ~0xFFFF;
    USART2->USART_BRR = CPU_FREQUENCY/9600;

    // Enabling the oversampling.
    USART2->USART_CR1 &= ~(1 << 15);

    // Setting the word length to 8 data bits,
    // and n stop bit.
    USART2->USART_CR1 &= ~(1 << 12);

    // We proceed then to enable the TX bit, that 
    // let's us actually transmit bits.
    // We do the same for the RX bit, so we can 
    // actually receive the bits and handle them.
    //
    // Section 19.6.4.
    USART2->USART_CR1 |= (1 << 3); // CR1[3], transmitter enable
    USART2->USART_CR1 |= (1 << 2); // CR1[2], receiver enable

    // We also set up the stop bit.
    USART2->USART_CR2 &= ~(3 << 12);

    // Finally, we can enable the USART2.
    //
    // Section 19.6.4
    //
    USART2->USART_CR1 |= (1 << 13); // CR1[13], USART enable.
}

void setup_systick() {
    // Enable Clock for SysTick (Section 6.3.12)
    RCC->RCC_APB2ENR |= (1 << 14);

    // We load the reload register with the maximum value (Section 4.4.2)
    SYST->SYST_RVR |= 0x00FFFFFF;

    // We set as internal source the processor clock, from where our systick will 'based' on, the 
    // 'rhythm' to derive from (Section 4.4.1).
    SYST->SYST_CSR |= (1 << 2);
    
    // We then proceed to finally enable the SysTick timer (Section 4.4.1)
    SYST->SYST_CSR |= (1 << 0);
}

void write_string(char *string, size_t len) {
    while (len > 0) {
        write_byte(*(uint8_t *) string++);
        len--;
    }
}

int main(void) {
    setup_gpio();
    setup_systick();
    setup_usart();

    while(1) {
        // We check each iteration if the timer has expired
        // in particular we check if the COUNTFLAG is 1, if so
        // it means that the timer counter to 0 since last time this was read.
        // (Section 4.4.1)
        //
        if (SYST->SYST_CSR & (1 << 16)) {
            send_ack();
            handle_packet();
        }
    }

    return 0;
}
