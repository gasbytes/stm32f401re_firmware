/**
 *@brief Startup Code for Vector Table Initialization and Reset Handling
 **/
#include <stdint.h>

/* 
 * Global variables, symbols taken
 * from the linker script to be 
 * initialized.
 * */
extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

/** Prototypes **/
extern int main(void);
void Reset_handler          (void);
extern void SysTick_Handler        (void);

/** Initialize Interrupt Vector **/
__attribute__ ((section(".isr_vector")))
void (* const fpn_vector[])(void) = {
    (void (*)(void))(&_estack),
    Reset_handler,
    0,
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0, 
    0,
    SysTick_Handler
};

void Reset_handler(void){
    /*
     * Copy .data from FLASH to SRAM, so we can actually apply read and write instructions to that memory. 
     * */
    uint32_t * pSRC = (uint32_t *)&_sidata;
    uint32_t * pDST = (uint32_t *)&_sdata;

    for(uint32_t *dataptr = (uint32_t *)pDST; dataptr < &_edata;){
        *dataptr++ = *pSRC++;
    }

    /*
     * We initialize the bss section with zeroes, since it containes 
     * all unitialized data.
     * */
    for(uint32_t *bss_ptr = (uint32_t *)&_sbss; bss_ptr < &_ebss;){
        *bss_ptr++ = 0;
    }

    /*
     * Call to the main function.
     * */
    main();
}
