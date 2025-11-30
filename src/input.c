/* input.c - Implementation of controller polling */

#include <stdint.h>
#include "input.h"

#define JOY1 (*(volatile uint8_t*)0x4016)

static uint8_t s_buttons = 0;
static uint8_t s_prev_buttons = 0;

/* Low-level NES pad read: 8 bits, one per button. */
static uint8_t read_joy1_raw(void) {
    uint8_t i;
    uint8_t buttons = 0;

    /* Strobe: latch physical buttons into shift register. */
    JOY1 = 1;
    JOY1 = 0;

    for (i = 0; i < 8u; ++i) {
        if (JOY1 & 1) {
            buttons |= (1u << i);
        }
    }
    return buttons;
}

void input_update(void) {
    s_prev_buttons = s_buttons;
    s_buttons = read_joy1_raw();
}

uint8_t input_get_buttons(void) {
    return s_buttons;
}

uint8_t input_get_prev_buttons(void) {
    return s_prev_buttons;
}

uint8_t input_pressed(uint8_t mask) {
    return (uint8_t)((s_buttons & mask) && !(s_prev_buttons & mask));
}

uint8_t input_held(uint8_t mask) {
    return (uint8_t)(s_buttons & mask);
}
