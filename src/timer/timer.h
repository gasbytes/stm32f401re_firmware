#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

/*
 * Struct of the (simple) timer.
 * */
typedef struct minimal_timer_t {
    // General time that the timer has to wait
    // before triggering the next event.
    uint32_t wait_time;
    // Actual time in systicks, to make the easier 
    // the reset and more system based).
    uint32_t target_time;
    // If it's true, reset the timer.
    int auto_reset;
} minimal_timer_t;

#endif // !TIMER_H
