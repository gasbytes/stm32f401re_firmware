/**
 *@brief simple blinky project
 **/
#include <stdint.h>
#include "../../inc/peripherals.h"

#define MODER 2
#define pin5 5

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

/**
 *@brief This is a simple delay function implementation, that waits for <time>ms.
 *
 *In this implementation the inner for loop, cycles for 1600 CLK Cycles which results in around 1ms delay, depending on the parameter <time>.
 *
 *@param[in] time | number of ms the processor should wait
 **/
void wait_ms(int time) {
    for(int i = 0; i < time; i++) { 
        for(int j = 0; j < 1600; j++);
    }
}

/**
 * @brief Main entry point for blinking project
 *
 * To see where the entry main function gets called
 * look at the file init.c in the folder init.
 *
 * GPIOA Peripherals are configured to OUTPUT, with LED connected to PA5 being toggled every 1000ms.
 **/
int main(void) {

    // Enable Clock for the GPIOA Peripheral (Section 6.3.9)
    // This is a OR operation and lets us set individual bits,
    // in particular in this case we set the least significant bit to 1.
    // Since to enable the clock for the GPIOA we need to set 
    // bit 0 to 1 (0x00000001).
    RCC->RCC_AHB1ENR |= 1;
    
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
    // This is a OR operation, in particular in this case...
    GPIOA->GPIOx_MODER |=  (1 << (pin5 * MODER));
    
    for(;;) {
        // Toggle PA5 (Section 8.4.6)
        GPIOA->GPIOx_ODR ^= (1 << pin5);

        // Wait for 100ms
        wait_ms(100);
    }
}
