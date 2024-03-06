/*
 *@brief simple redirect printf to uart project
 **/
#include "../../inc/peripherals.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define PA2 2
#define PA3 3

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

void setup_gpio() {
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
    // We also do the same for the RX.
    GPIOA->GPIOx_AFRL &= ~(0xF << (PA2 * 4)); // clear bits AFRL7[3:0]
    GPIOA->GPIOx_AFRL |= (7 << (PA2 * 4)); // set bits AFRL7[3:0]
    GPIOA->GPIOx_AFRL &= ~(0xF << (PA3 * 4));
    GPIOA->GPIOx_AFRL |=  (7   << (PA3 * 4));
    
    
    // Now we can enable the alternate function, by setting the moder
    // register to 10 (binary).
    //
    // Section 8.4.9.
    GPIOA->GPIOx_MODER &= ~(3 << (PA2 * 2)); // clear bits MODER1[1:0]
    GPIOA->GPIOx_MODER |= (2 << (PA2 * 2)); // set bits MODER1[1:0]
    GPIOA->GPIOx_MODER &= ~(3 << (PA3 * 2)); // clear bits MODER1[1:0]
    GPIOA->GPIOx_MODER |= (2 << (PA3 * 2)); // set bits MODER1[1:0]
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
    USART2->USART_BRR = CPU_FREQUENCY/9600;

    // We proceed then to enable the TX bit, that 
    // let's us actually transmit bits.
    // We do the same for the RX bit.
    //
    // Section 19.6.4.
    USART2->USART_CR1 |= (1 << 3); // CR1[3], transmitter enable
    USART2->USART_CR1 |= (1 << 2); // CR1[2], receiver enable
    USART2->USART_CR1 &= ~(1 << 12);

    // Finally, we can enable the USART2.
    //
    // Section 19.6.4
    //
    USART2->USART_CR1 |= (1 << 13); // CR1[13], USART enable.
}

char read_byte();

int main(void) {
    setup_gpio();
    setup_usart();

    int a;
    char input[10], byte;

    while(1) {
        for(int i = 0; i < 10; i++) input[i++] = '\0';
        for(int i = 0; (byte = read_byte()) != '\r'; ) input[i++] = byte;
        sscanf(input, "%d", &a);

        printf("input typed: %d\n", a) ;
        printf("\n");
    }

    return 0;
}
