/* input.h - NES controller 1 polling */

#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

/* Button bit masks (lowest bit = A, then B, Select, Start, Up, Down, Left, Right). */
enum {
    BTN_A      = 0x01,
    BTN_B      = 0x02,
    BTN_SELECT = 0x04,
    BTN_START  = 0x08,
    BTN_UP     = 0x10,
    BTN_DOWN   = 0x20,
    BTN_LEFT   = 0x40,
    BTN_RIGHT  = 0x80
};

/* Call once per frame to poll controller 1. */
void input_update(void);

/* Current and previous button state snapshots. */
uint8_t input_get_buttons(void);
uint8_t input_get_prev_buttons(void);

/* Helper queries */
uint8_t input_pressed(uint8_t mask);  /* went from up to down this frame */
uint8_t input_held(uint8_t mask);     /* currently down */

#endif
