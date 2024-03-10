#include "../../inc/peripherals.h"
#include "p_p.h"
#include <stddef.h>
#include <stdint.h>

#define PA2 2
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

int main(void) {
    setup_gpio();
    setup_systick();
    setup_usart();

    // Create a packet for testing
    uint8_t test_data1[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    packet_t test_packet = *create_packet(sizeof(test_data1), test_data1);

    // Create an ACK packet
    uint8_t test_data2[] = {ACK};
    packet_t ack_packet = *create_packet(sizeof(test_data2), test_data2);

    // Create an ACK packet
    uint8_t test_data3[] = {RCK};
    packet_t rck_packet = *create_packet(sizeof(test_data3), test_data3);

    while(1) {
        // We check each iteration if the timer has expired
        // in particular we check if the COUNTFLAG is 1, if so
        // it means that the timer counter to 0 since last time this was read.
        // (Section 4.4.1)
        //
        if (SYST->SYST_CSR & (1 << 16)) {
            printf("Sending test_packet:\n");
            print_packet(&test_packet);
            print_packet(&ack_packet);
            print_packet(&rck_packet);
            send_packet(&test_packet);
            send_rck();
            send_ack();
        }
    }

    return 0;
}
