/**
 *@brief simple i2c project
 **/
#include <stdint.h>
#include "../../inc/peripherals.h"

#define MODER 2
#define PA5 5
#define PA8 8
#define PA9 9
#define CR1_SWRST       15
#define CR1_ACK         10
#define CR1_STOP         9
#define CR1_START        8
#define CR1_PE           0
#define SR1_TxE         7
#define SR1_RxNE        6
#define SR1_BTF         2
#define SR1_ADDR        1
#define SR1_SB          0
#define SR2_BUSY        1

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
 * @brief Struct Pointer for GPIOB Peripherals assigned with fixed address specified in reference manual.
 *
 * See Memory map, Section 2.3.
 **/
GPIOx_t * const GPIOB = (GPIOx_t  *)  (0x40020000 + 0x400);

/**
 * @brief Struct Pointer for I2C1 Peripherals assigned with fixed address specified in reference manual.
 *
 * See Memory map, Section 2.3.
 **/
I2Cx_t * const I2C1 = (I2Cx_t *)  (0x40020000 + 0x400);

/*
 * @brief Struct Pointer for SYST (System Timer) assigned with fixed address specified in the datasheet.
 *
 * See section 4.4 System timer, SysTick (ARM-cortex-m4 datasheet).
 * */
SYST_t * const SYST = (SYST_t *) 0xE000E010;


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
    GPIOA->GPIOx_MODER &= ~(3 << (PA5 * MODER));

    // Write Value 1 to MODER Bitfield PA5 to configure PA5 as OUTPUT (Section 8.4.1)
    // This is a OR operation, in particular in this case we set to 1 the pin5 (01).
    GPIOA->GPIOx_MODER |=  (1 << (PA5 * MODER));
}

void setup_i2c_pullup(void) {
    // Enable Clock for the GPIOA Peripheral (Section 6.3.9)
    // This is a OR operation and lets us set individual bits,
    // in particular in this case we set the least significant bit to 1.
    // Since to enable the clock for the GPIOA we need to set 
    // bit 0 to 1 (0x00000001).
    RCC->RCC_AHB1ENR |= 1;


    // We then proceed to clear, and then set the PA8 and PA9
    // pins as OUTPUT.
    // We will use this ones later on to set them as open-drain and the pull up.
    // The pins A8 and A9 are arbitrary chosen.
    GPIOA->GPIOx_MODER &= ~(3 << (PA8 * MODER));
    GPIOA->GPIOx_MODER &= ~(3 << (PA9 * MODER));
    GPIOA->GPIOx_MODER |= (1 << (PA8 * MODER));
    GPIOA->GPIOx_MODER |= (1 << (PA9 * MODER));


    // In I2C communication, both SDA (data line) and SCL (clock line) are open-drain or open-collector. 
    // This means that the devices connected to these lines can only pull the line low (to ground), but 
    // cannot drive it high (to Vcc) actively.
    // Pull-up resistors ensure that the lines are in a defined state when no devices are actively pulling them low. 
    // They pull the lines up to the high state (Vcc) when no active device is pulling them low.
    // This helps prevent floating states and ensures proper signaling between devices.
    //
    // The register to configure for this action is the PUPDR register, in particular
    // we clear and enable the equivalent pin that we used for the MODER register.
    // Section 8.4.4.
    GPIOA->GPIOx_PUPDR &= ~(3 << (PA8 * 2));
    GPIOA->GPIOx_PUPDR &= ~(3 << (PA9 * 2));
    GPIOA->GPIOx_PUPDR |=  (1 << (PA8 * 2));
    GPIOA->GPIOx_PUPDR |=  (1 << (PA9 * 2));

    // Open-drain configuration allows multiple devices to share the same communication lines without conflict.
    // Each device can pull the line low, but none of them actively drive it high. The pull-up resistor ensures 
    // that the line is pulled high when no device is pulling it low.
    // This allows for bidirectional communication and facilitates multi-master communication in I2C.
    //
    // For this, we use the OTYPER register, and we enable the equivalent pins.
    // Section 8.4.2.
    GPIOA->GPIOx_OTYPER |= (1 << PA8);
    GPIOA->GPIOx_OTYPER |= (1 << PA9);

    // Finally we set the pins PA8 and PA9 to HIGH.
    // Setting the pins to output mode ensures that the microcontroller can control the state of the pins.
    // Setting the pins to a high state ensures that they are initially pulled up by the pull-up resistors, 
    // establishing the idle state of the communication lines.
    GPIOA->GPIOx_ODR |= (1 << PA8);
    GPIOA->GPIOx_ODR |= (1 << PA9);
}

void setup_i2c(void) {
    // We enable the clock needed to use the 
    // GPIOB peripheral.
    // We also enable the I2C clock for the same reason.
    // Section 6.3.9 and Section 6.3.11
    RCC->RCC_AHB2ENR |= 2;
    RCC->RCC_APB1ENR |= (1 << 21);

    // We configure the GPIOB the same we did for the GPIOA, so
    // we configure the alternate functions following the 
    // alternate function map, followed by
    // setting the OpenDrain and the pull up resistors.
    // We do this practice two times since it's basically the common
    // practice for I2C.
    // We set up two pins for data transmission: one for the data line and 
    // one for the clock line.
    GPIOB->GPIOx_AFRH &= ~0xFF;             
    GPIOB->GPIOx_AFRH |= 0x44;                                        
    GPIOB->GPIOx_MODER &= ~(3 << (2 * PA8)); 
    GPIOB->GPIOx_MODER &= ~(3 << (2 * PA9));
    GPIOB->GPIOx_MODER |= (2 << (2 * PA8));
    GPIOB->GPIOx_MODER |= (2 << (2 * PA9));

    GPIOB->GPIOx_OTYPER |= (1 << PA8);      
    GPIOB->GPIOx_OTYPER |= (1 << PA9);        
    GPIOB->GPIOx_PUPDR &= ~(0xF << 16);  
    GPIOB->GPIOx_PUPDR |=  (1 << (2 * PA8));     
    GPIOB->GPIOx_PUPDR |=  (1 << (2 * PA9));     

    // Finally we setup the specifics for the I2C peripheral.
    // The first thing that we do is to reset the I2C, by setting it into reset
    // state, by setting the 16th bit to 1, of I2C control register 1.
    // The second set is to deactivate it, right after resetting.
    // Section 18.6.1.
    //
    // The next step, is to set the frequency of the clock line,
    // to do that, we are gonna use the I2C control register 2, 
    // by controlling and setting the bits FREQ[0:5].
    // We are gonna set it to 16MHz.
    // Section 18.6.2.
    //
    // The next step is to just set up the standard mode of 
    // the data line, which is the initial transfer speed mode
    // of the I2C specification.
    // We are gonna set it up to maximum available: 100kHz.
    // Section 18.6.8.
    //
    // After setting up the maximum rise time (Section 18.6.9) which is 
    // basically the time taken for the line to climb from LOW to HIGH 
    // (measured in ns), we finally enable the I2C1 module.
    I2C1->I2C_CR1 = (1 << 15);                  
    I2C1->I2C_CR1 &= ~(1 << 15);               
    I2C1->I2C_CR2 = (1 << 4);                 
    I2C1->I2C_CCR = 80;                         
    I2C1->I2C_TRISE = 17;                       // Set maximum rise time
    I2C1->I2C_CR1 |= 1;                         // Enable I2C1 Module
}

// Function to write a byte to I2C bus
// The section used for this function are all under
// the main I2C section, so the section 18.
int I2C1_byte_read(char slave_addr, char mem_addr, uint8_t *data){
    volatile int tmp;

    // Wait until the I2C bus is not busy
    while(I2C1->I2C_SR2 & (1 << SR2_BUSY));

    // Generate a Start Signal to initiate communication
    I2C1->I2C_CR1 |= (1 << CR1_START);

    // Wait until the Start Bit is set, indicating that the Start Signal has been successfully transmitted
    while(!(I2C1->I2C_SR1 & (1 << SR1_SB)));

    // Transmit the Slave Address along with the Write bit (SLA+W)
    I2C1->I2C_DR = slave_addr << 1;

    // Wait for the Address Acknowledge (ACK) from the slave device
    while(!(I2C1->I2C_SR1 & (1 << SR1_ADDR)));
    tmp = I2C1->I2C_SR2;  // Clear ADDR Bit by reading SR2

    // Wait for the Transmit Data Register (DR) to be empty to send the memory address
    while(!(I2C1->I2C_SR1 & (1 << SR1_TxE)));
    // Send the Memory Address to read from
    I2C1->I2C_DR = mem_addr;
    // Wait for the Transmit Data Register (DR) to be empty again
    while(!(I2C1->I2C_SR1 & (1 << SR1_TxE)));

    // Generate a Restart Signal to switch from Write to Read mode
    I2C1->I2C_CR1 |= (1 << CR1_START);
    // Wait until the Start Bit is set, indicating that the Restart Signal has been successfully transmitted
    while(!(I2C1->I2C_SR1 & 1));

    // Transmit the Slave Address along with the Read bit (SLA+R)
    I2C1->I2C_DR = slave_addr << 1 | 1;
    // Check if ACK received from the slave device
    while(!(I2C1->I2C_SR1 & (1 << SR1_ADDR)));
    // Disable Acknowledge (ACK) from the master to indicate the end of data reception
    I2C1->I2C_CR1 &= ~(1 << CR1_ACK);
    tmp = I2C1->I2C_SR2;  // Clear ADDR FLAG by reading SR2

    // Generate a Stop Signal to end the communication
    I2C1->I2C_CR1 |= (1 << CR1_STOP);

    // Wait until the Receive Data Register (DR) is not empty
    while(!(I2C1->I2C_SR1 & (1 << SR1_RxNE)));
    // Read the received data from the Receive Data Register (DR)
    *data = I2C1->I2C_DR;

    return 0;
}

// Function to write a byte to the I2C bus.
// Like for the byte_read, all the stuff used here
// is documentaed inside the section 18 of the MCU's datasheet.
int I2C1_byte_write(char slave_addr, char mem_addr, uint8_t data){
    volatile int tmp;

    // Wait until the I2C bus is not busy
    while(I2C1->I2C_SR2 & (1 << SR2_BUSY));

    // Generate a Start Signal to initiate communication
    I2C1->I2C_CR1 |= (1 << CR1_START);

    // Wait until the Start Bit is set, indicating that the Start Signal has been successfully transmitted
    while(!(I2C1->I2C_SR1 & (1 << SR1_SB)));

    // Send the Slave Address along with the Write bit (SLA+W)
    I2C1->I2C_DR = slave_addr << 1;

    // Wait until the Address Acknowledge (ACK) from the slave device
    while(!(I2C1->I2C_SR1 & (1 << SR1_ADDR)));
    tmp = I2C1->I2C_SR2;  // Clear ADDR Bit by reading SR2

    // Wait until the Transmit Data Register (DR) is empty to send the memory address
    while(!(I2C1->I2C_SR1 & (1 << SR1_TxE)));

    // Send the Memory Address to write to
    I2C1->I2C_DR = mem_addr;

    // Wait until the Transmit Data Register (DR) is empty to send the data
    while(!(I2C1->I2C_SR1 & (1 << SR1_TxE)));
    // Send the data to be written
    I2C1->I2C_DR = data;

    // Wait until all the Transmit Bytes have been sent
    while(!(I2C1->I2C_SR1 & (1 << SR1_BTF)));
    // Generate a Stop Signal to end the communication
    I2C1->I2C_CR1 |= (1 << CR1_STOP);

    return 0;
}

void setup_systick(void) {
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

/**
 * @brief Main entry of the i2c project.
 **/
int main(void) {
    setup_gpio();
    setup_systick();
    setup_i2c_pullup();
    setup_i2c();

    uint8_t a, b;

    I2C1_byte_read(0x40, 0x0F, &a);
    I2C1_byte_write(0x40, 0x2E, 0x84);

    while (1) {
        I2C1_byte_read(0x40, 0x10, &a);
        I2C1_byte_read(0x40, 0x11, &b);
    
        if (SYST->SYST_CSR & (1 << 16)) {
            GPIOA->GPIOx_ODR ^= (1 << PA5);
        }
    }
}
