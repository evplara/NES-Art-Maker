/* main.c - NES Paint: step 1-2
   - Module structure set up
   - RAM canvas rendered as a static pattern
*/

#include <stdint.h>
#include "ppu.h"
#include "input.h"
#include "canvas.h"

/* Simple background palette:
 * - slot 0: universal background
 * - slots 1-3: colors used by tiles 1-3
 * - remaining palettes are unused for now
 */
static const uint8_t s_bg_palette[16] = {
    0x0F, 0x01, 0x11, 0x21,   /* palette 0: background + 3 blues */
    0x0F, 0x06, 0x16, 0x26,   /* palette 1 (unused) */
    0x0F, 0x09, 0x19, 0x29,   /* palette 2 (unused) */
    0x0F, 0x0A, 0x1A, 0x2A    /* palette 3 (unused) */
};

void main(void) {
    /* 1) Initialize PPU (scroll, background on, sprites off). */
    ppu_init();

    /* 2) Load background palette to $3F00. */
    ppu_wait_vblank();
    ppu_load_bg_palette(s_bg_palette, 16);

    /* 3) Prepare a demo pattern in the RAM canvas. */
    canvas_fill_demo_pattern();

    /* 4) Upload entire canvas as tiles to nametable 0. */
    ppu_wait_vblank();
    canvas_render_full();

    /* 5) Main loop: keep the picture stable and poll input.
       We'll use input in the next step for cursor/tools. */
    for (;;) {
        ppu_wait_vblank();
        input_update();
        /* No drawing logic yet; screen stays static. */
    }
}
