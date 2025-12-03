/* canvas.h - RAM-backed "pixel art" canvas */

#ifndef CANVAS_H
#define CANVAS_H

#include <stdint.h>

#define CANVAS_WIDTH  32
#define CANVAS_HEIGHT 30

/* Each entry is a small color index 0..3 for now. */
extern uint8_t g_canvas[CANVAS_HEIGHT][CANVAS_WIDTH];

/* Clear the entire canvas to one color index. */
void canvas_clear(uint8_t color);

/* Fill canvas with a demo pattern (for testing the pipeline). */
void canvas_fill_demo_pattern(void);

void canvas_set_pixel(uint8_t x, uint8_t y, uint8_t color);

/* Upload full canvas to nametable 0 ($2000) as tiles. */
void canvas_render_full(void);
void canvas_render_tile(uint8_t x, uint8_t y);
void canvas_render_rows(uint8_t y_start, uint8_t row_count);

#endif
