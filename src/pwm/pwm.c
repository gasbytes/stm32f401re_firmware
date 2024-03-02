/**
 *@brief simple pwm project
 **/
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

/*
 * @brief Struct Pointer for TIM2 Peripherals assigned with fixed address specified in reference manual.
 * */
TIMx_t  * const TIM2 = (TIMx_t *) 0x40000000;

/*
 * @brief Struct Pointer for SYST (System Timer) assigned with fixed address specified in the datasheet.
 *
 * See section 4.4 System timer, SysTick (ARM-cortex-m4 datasheet).
 * */
SYST_t * const SYST = (SYST_t *) 0xE000E010;

void set_duty_cycle(float duty_cycle) {
    const float raw_value = (float)1000 * (duty_cycle / 100.0f);

    // set duty cycle on channel 1
    TIM2->TIMx_CCR1 = (uint32_t)raw_value;
}

void setup_gpio(void) {
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
    // Write Value 2 (10) to MODER Bitfield PA5 to configure PA5 as alternate function (Section 8.4.1)
    GPIOA->GPIOx_MODER |=  (2 << (pin5 * MODER));
    // Set PA5 AF01 (Section 8.4.9)
    GPIOA->GPIOx_AFRL &= ~(0xF << (pin5 * 4));
    GPIOA->GPIOx_AFRL |=  (1 << (pin5 * 4));
}

void setup_tim(void) {
    // Enable the clock for the timer peripheral, in particular we are gonna use the 
    // the TIM2 peripheral (Section 6.3.11), which is at bit 1.
    // In particular we use the TIM2 because the alternate function table (table 9 in the stm32f401re.pdf)
    // tells us that the PA5 is mapped to AF01, and tells us we can drive it with TIM2_CH1/TIM2_ETR.
    RCC->RCC_APB1ENR |= 1;
    // fCK_PSC / (PSC[15:0] + 1)
    // 84 Mhz / 84 * 1000 = 1khz timer clock speed
    TIM2->TIMx_PSC = 84 - 1;
    // set period
    TIM2->TIMx_ARR = 1000 - 1;
    // Enable channel 1 in capture/compare register
    // Set oc1 mode as pwm (0b110 or 0x6 in bits 6-4)
    TIM2->TIMx_CCMR1 |= (0x6 << 4);
    // enable oc1 preload bit 3
    TIM2->TIMx_CCMR1 |= (1 << 3);
    // enable capture/compare ch1 output
    TIM2->TIMx_CCER |= (1 << 0);
    // Enable timer 2 module
    TIM2->TIMx_CR1 |= 1;
}

void setup_syst(void) {
    // We load the reload register with the maximum value (Section 4.4.2)
    SYST->SYST_RVR = 250000 - 1;

    // We set as internal source the processor clock, from where our systick will 'based' on, the 
    // 'rhythm' to derive from (Section 4.4.1).
    SYST->SYST_CSR |= (1 << 2);
    
    // We then proceed to finally enable the SysTick timer (Section 4.4.1)
    SYST->SYST_CSR |= (1 << 0);
}

/**
 * @brief Main entry point for pwm project
 *
 * Period: how much time passes between a peak of a wave and the next wave,
 * in particular it can be calculated with 1/f.
 *
 * Duty cycle: how long is the wave in positive state.
 *
 * The implementation of it it's quite simple:
 *
 * counter register -> counts till the auto-reload value.
 * auto-reload register -> max value we can reach.
 * capture compare value -> value we compare to.
 * This comparison with the capture compare value allows us 
 * to create a wave.
 *
 * So basically, starting from a digital source (5v or 0v), we 
 * generate a fake analogue more precise voltage (e.g. 0.3v).
 * By cycling a digital signal ON and OFF at a fast enough rate
 * and at a certain duty cycle, the output will appear like a constant 
 * voltage analouge signal.
 * Which give this "fading effect" to our eyes, but in reality, we are just
 * switching it on and off really quickly.
 **/
int main(void) {
    setup_gpio();
    setup_tim();
    setup_syst();

    float duty_cycle = 0.0f;
    set_duty_cycle(duty_cycle);

    while (1) {
        // We check each iteration if the timer has expired
        // in particular we check if the COUNTFLAG is 1, if so
        // it means that the timer counter to 0 since last time this was read.
        //
        // If we are inside the if scope, we just increment the duty cycle, to increment
        // its brightness (increment how long it stays on).
        if (SYST->SYST_CSR & (1 << 16)) {
            duty_cycle += 1.0f;

            if (duty_cycle > 100.0f) {
                duty_cycle = 0.0f;
            }

            set_duty_cycle(duty_cycle);
        }
    }

    return 0;
}
