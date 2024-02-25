#include "../inc/peripherals.h"

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

/**
 * @brief Main entry point for blinking project
 *
 * To see where the entry main function gets called
 * look at the file init.c in the folder init.
 *
 * Basically the same thing as the the blinky program, but instead we 
 * use the SystemTimer, by setting up and then check the value of the 
 * register CSR, since it's the control and status register of the SYST.
 * The value tells us if a new clock cycle as just began (reset of the
 * variable with SYST_RVR).
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


    // Enable Clock for SysTick (Section 6.3.12)
    RCC->RCC_APB2ENR |= (1 << 14);

    // We load the reload register with the maximum value (Section 4.4.2)
    SYST->SYST_RVR |= 0x00FFFFFF;

    // We set as internal source the processor clock, from where our systick will 'based' on, the 
    // 'rhythm' to derive from (Section 4.4.1).
    SYST->SYST_CSR |= (1 << 2);
    
    // We then proceed to finally enable the SysTick timer (Section 4.4.1)
    SYST->SYST_CSR |= (1 << 0);

    while (1) {
        // We check each iteration if the timer has expired
        // in particular we check if the COUNTFLAG is 1, if so
        // it means that the timer counter to 0 since last time this was read,
        // toggle the led! (Section 4.4.1)
        if (SYST->SYST_CSR & (1 << 16)) {
            // Toggle PA5 (Section 8.4.6)
            GPIOA->GPIOx_ODR ^= (1 << pin5);
        }
    }
}
