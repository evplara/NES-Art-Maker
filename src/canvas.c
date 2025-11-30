/* canvas.c - Implementation of the RAM canvas and nametable upload */

#include <stdint.h>
#include "canvas.h"
#include "ppu.h"

/* Global canvas buffer in work RAM ($6000-$7FFF via nes.cfg BSS). */
uint8_t g_canvas[CANVAS_HEIGHT][CANVAS_WIDTH];

void canvas_clear(uint8_t color) {
    uint8_t x, y;

    for (y = 0; y < CANVAS_HEIGHT; ++y) {
        for (x = 0; x < CANVAS_WIDTH; ++x) {
            g_canvas[y][x] = color;
        }
    }
}

/* Simple 4-color stripe/checker pattern so we SEE something. */
void canvas_fill_demo_pattern(void) {
    uint8_t x, y;

    for (y = 0; y < CANVAS_HEIGHT; ++y) {
        for (x = 0; x < CANVAS_WIDTH; ++x) {
            g_canvas[y][x] = (uint8_t)(((x >> 1) + (y >> 1)) & 3u);
        }
    }
}

/* Convert the canvas array into tiles in nametable 0 ($2000).
   For now:
   - tile index == color index (0..3)
   - attribute table left at 0 (all use background palette 0)
*/
void canvas_render_full(void) {
    uint8_t x, y;
    uint16_t addr = 0x2000;

    /* Start at top-left of nametable 0. */
    PPUADDR = (uint8_t)(addr >> 8);
    PPUADDR = (uint8_t)(addr & 0xFF);

    for (y = 0; y < CANVAS_HEIGHT; ++y) {
        for (x = 0; x < CANVAS_WIDTH; ++x) {
            uint8_t color = g_canvas[y][x];
            uint8_t tile  = color;  /* 0..3 -> tile #0..3 */
            PPUDATA = tile;
        }
    }

    /* Attribute table ($23C0-$23FF) will remain all zeros (palette 0).
       We'll tweak attributes later if we want fancier palette usage. */
}
