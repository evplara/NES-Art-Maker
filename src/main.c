#include <stdint.h>
#include "ppu.h"
#include "input.h"
#include "canvas.h"
#define UI_PALETTE_ROW      0
#define UI_HIGHLIGHT_ROW    1
#define UI_FIRST_DRAW_ROW   2

/* Tools */
typedef enum {
    TOOL_BRUSH = 0,
    TOOL_ERASER,
    TOOL_LINE,
    TOOL_RECT,
    TOOL_FILL,
    TOOL_COUNT
} Tool;

/* Flood fill queue */
#define FILL_QUEUE_MAX (CANVAS_WIDTH * CANVAS_HEIGHT)

typedef struct {
    uint8_t x;
    uint8_t y;
} FillNode;

static FillNode s_fill_queue[FILL_QUEUE_MAX];
static uint8_t s_full_redraw_needed = 0;


/* Globals for tools */
static Tool    s_current_tool = TOOL_BRUSH;
static uint8_t s_drag_active = 0;
static uint8_t s_drag_start_x = 0;
static uint8_t s_drag_start_y = 0;

/* Background palette:
 * palette 0: universal + three colors used by tiles 1-3.
 */
static const uint8_t s_bg_palette[16] = {
    0x0F, 0x01, 0x11, 0x21,   /* palette 0: background + 3 blues */
    0x0F, 0x06, 0x16, 0x26,   /* palette 1 (unused) - reds */
    0x0F, 0x09, 0x19, 0x29,   /* palette 2 (unused) - greens */
    0x0F, 0x0A, 0x1A, 0x2A    /* palette 3 (unused) */
};

/* Palette bar at top:
   - row 0, x=0..3: swatches 0..3
   - row 1, x=0..3: highlight under current_color
*/
static void ui_update_palette_bar(uint8_t current_color) {
    uint8_t x;

    /* Row 0: swatches */
    for (x = 0; x < 4u; ++x) {
        g_canvas[UI_PALETTE_ROW][x] = x;            /* tile 0..3 */
        canvas_render_tile(x, UI_PALETTE_ROW);
    }

    /* Row 1: clear highlight row */
    for (x = 0; x < 4u; ++x) {
        g_canvas[UI_HIGHLIGHT_ROW][x] = 0;
        canvas_render_tile(x, UI_HIGHLIGHT_ROW);
    }

    /* Highlight under current_color */
    g_canvas[UI_HIGHLIGHT_ROW][current_color] = current_color;
    canvas_render_tile(current_color, UI_HIGHLIGHT_ROW);
}

/* Simple tool indicator in row 1, columns 6..9.
   We just use different color patterns for each tool:
   - Brush: all tile 1 (blue)
   - Eraser: all tile 0 (black)
   - Line: all tile 2 (mid blue)
   - Rect: all tile 3 (light blue)
   - Fill: pattern 1,0,1,0
*/
static void ui_update_tool_indicator(Tool tool) {
    uint8_t x;
    uint8_t pattern[4];

    switch (tool) {
        case TOOL_BRUSH:
            pattern[0] = pattern[1] = pattern[2] = pattern[3] = 1;
            break;
        case TOOL_ERASER:
            pattern[0] = pattern[1] = pattern[2] = pattern[3] = 0;
            break;
        case TOOL_LINE:
            pattern[0] = pattern[1] = pattern[2] = pattern[3] = 2;
            break;
        case TOOL_RECT:
            pattern[0] = pattern[1] = pattern[2] = pattern[3] = 3;
            break;
        case TOOL_FILL:
        default:
            pattern[0] = 1;
            pattern[1] = 0;
            pattern[2] = 1;
            pattern[3] = 0;
            break;
    }

    for (x = 0; x < 4u; ++x) {
        uint8_t cx = (uint8_t)(6u + x);
        g_canvas[UI_HIGHLIGHT_ROW][cx] = pattern[x];
        canvas_render_tile(cx, UI_HIGHLIGHT_ROW);
    }
}

/* Draw a line from (x0,y0) to (x1,y1) using Bresenham. */
static void draw_line(uint8_t x0, uint8_t y0,
                      uint8_t x1, uint8_t y1,
                      uint8_t color) {
    int ix0 = x0;
    int iy0 = y0;
    int ix1 = x1;
    int iy1 = y1;

    int dx = (ix1 > ix0) ? (ix1 - ix0) : (ix0 - ix1);
    int sx = (ix0 < ix1) ? 1 : -1;
    int dy = (iy1 > iy0) ? (iy0 - iy1) : (iy1 - iy0); /* negative */
    int sy = (iy0 < iy1) ? 1 : -1;
    int err = dx + dy;

    for (;;) {
        if (iy0 >= UI_FIRST_DRAW_ROW && iy0 < CANVAS_HEIGHT &&
            ix0 >= 0 && ix0 < CANVAS_WIDTH) {
            canvas_set_pixel((uint8_t)ix0, (uint8_t)iy0, color);
            /* canvas_render_tile((uint8_t)ix0, (uint8_t)iy0); no VRAM write bc NES is slow*/
        }

        if (ix0 == ix1 && iy0 == iy1) {
            break;
        }

        {
            int e2 = err << 1;
            if (e2 >= dy) { err += dy; ix0 += sx; }
            if (e2 <= dx) { err += dx; iy0 += sy; }
        }
    }
}

/* Filled rectangle between (x0,y0) and (x1,y1). */
static void draw_rect(uint8_t x0, uint8_t y0,
                      uint8_t x1, uint8_t y1,
                      uint8_t color) {
    uint8_t x, y;
    uint8_t x_min = (x0 < x1) ? x0 : x1;
    uint8_t x_max = (x0 < x1) ? x1 : x0;
    uint8_t y_min = (y0 < y1) ? y0 : y1;
    uint8_t y_max = (y0 < y1) ? y1 : y0;

    if (y_min < UI_FIRST_DRAW_ROW) {
        y_min = UI_FIRST_DRAW_ROW;
    }
    if (y_max >= CANVAS_HEIGHT) {
        y_max = (uint8_t)(CANVAS_HEIGHT - 1u);
    }

    for (y = y_min; y <= y_max; ++y) {
        for (x = x_min; x <= x_max; ++x) {
            if (x < CANVAS_WIDTH) {
                canvas_set_pixel(x, y, color);
                /* canvas_render_tile(x,y); no VRAM write bc NES is slow*/
            }
        }
    }
}

/* Flood fill starting at (start_x, start_y) replacing target_color with new_color. */
static void tool_flood_fill(uint8_t start_x, uint8_t start_y, uint8_t new_color) {
    uint8_t target_color;
    uint16_t head = 0;
    uint16_t tail = 0;

    if (start_x >= CANVAS_WIDTH || start_y >= CANVAS_HEIGHT) {
        return;
    }
    if (start_y < UI_FIRST_DRAW_ROW) {
        return; /* don't flood UI rows */
    }

    target_color = g_canvas[start_y][start_x];
    if (target_color == new_color) {
        return;
    }

    s_fill_queue[tail].x = start_x;
    s_fill_queue[tail].y = start_y;
    tail++;

    while (head < tail && head < FILL_QUEUE_MAX) {
        uint8_t x = s_fill_queue[head].x;
        uint8_t y = s_fill_queue[head].y;
        head++;

        if (x >= CANVAS_WIDTH || y >= CANVAS_HEIGHT) {
            continue;
        }
        if (y < UI_FIRST_DRAW_ROW) {
            continue;
        }
        if (g_canvas[y][x] != target_color) {
            continue;
        }

        /* Paint this cell */
        g_canvas[y][x] = new_color;
        /* canvas_render_tile(x,y); no VRAM write bc NES is slow*/

        /* Enqueue neighbors that still have target_color */
        if (x > 0) {
            uint8_t nx = (uint8_t)(x - 1u);
            if (g_canvas[y][nx] == target_color && tail < FILL_QUEUE_MAX) {
                s_fill_queue[tail].x = nx;
                s_fill_queue[tail].y = y;
                tail++;
            }
        }
        if (x + 1u < CANVAS_WIDTH) {
            uint8_t nx = (uint8_t)(x + 1u);
            if (g_canvas[y][nx] == target_color && tail < FILL_QUEUE_MAX) {
                s_fill_queue[tail].x = nx;
                s_fill_queue[tail].y = y;
                tail++;
            }
        }
        if (y > UI_FIRST_DRAW_ROW) {
            uint8_t ny = (uint8_t)(y - 1u);
            if (g_canvas[ny][x] == target_color && tail < FILL_QUEUE_MAX) {
                s_fill_queue[tail].x = x;
                s_fill_queue[tail].y = ny;
                tail++;
            }
        }
        if (y + 1u < CANVAS_HEIGHT) {
            uint8_t ny = (uint8_t)(y + 1u);
            if (g_canvas[ny][x] == target_color && tail < FILL_QUEUE_MAX) {
                s_fill_queue[tail].x = x;
                s_fill_queue[tail].y = ny;
                tail++;
            }
        }
    }
}

/* Clear only the drawing area (rows >= UI_FIRST_DRAW_ROW). */
static void canvas_clear_drawing_area(void) {
    uint8_t x, y;

    for (y = UI_FIRST_DRAW_ROW; y < CANVAS_HEIGHT; ++y) {
        for (x = 0; x < CANVAS_WIDTH; ++x) {
            g_canvas[y][x] = 0;
            /* canvas_render_tile(x,y); no VRAM write bc NES is slow*/
        }
    }
}


void main(void) {
    uint8_t cursor_x;
    uint8_t cursor_y;
    uint8_t current_color = 1;  

    cursor_x = CANVAS_WIDTH  / 2;
    cursor_y = (CANVAS_HEIGHT + UI_FIRST_DRAW_ROW) / 2;

    /* Init PPU with rendering OFF */
    ppu_init();

    /*Load palette, clear canvas, set up UI */
    ppu_wait_vblank();
    ppu_load_bg_palette(s_bg_palette, 16);

    canvas_clear(0);                    /* clear entire canvas RAM */
    ui_update_palette_bar(current_color);
    ui_update_tool_indicator(s_current_tool);

    /*Upload full canvas once while rendering is OFF */
    canvas_render_full();

    /* Clear sprites and show initial cursor sprite */
    ppu_clear_oam();
    {
        uint8_t sprite_x = (uint8_t)(cursor_x * 8u);
        uint8_t sprite_y = (uint8_t)(cursor_y * 8u);
        ppu_draw_cursor_sprite(3, sprite_x, sprite_y);
    }

    /*  Turn on background + sprites */
    PPUCTRL = 0x00;
    PPUMASK = 0x18;   /* BG + sprites */

    /* Main loop */
    for (;;) {
        uint8_t sprite_x;
        uint8_t sprite_y;
        uint8_t buttons;
        uint8_t prev;

        ppu_wait_vblank();
        input_update();

        buttons = input_get_buttons();
        prev    = input_get_prev_buttons();

        /* Reset: START + SELECT together*/
        if ((buttons & BTN_START) && (buttons & BTN_SELECT) &&
            !(prev & BTN_START) && !(prev & BTN_SELECT)) {
            s_drag_active = 0;
            canvas_clear_drawing_area();
            /* UI still lives in rows 0-1; refresh bars just in case */
            ui_update_palette_bar(current_color);
            ui_update_tool_indicator(s_current_tool);
            s_full_redraw_needed = 1;
        } else {
            /* Tool selection: START cycles tool */
            if (input_pressed(BTN_START)) {
                s_drag_active = 0;
                s_current_tool = (Tool)((s_current_tool + 1) % TOOL_COUNT);
                ui_update_tool_indicator(s_current_tool);
            }

            /* color selection: SELECT cycles brush color 1..3- */
            if (input_pressed(BTN_SELECT)) {
                current_color++;
                if (current_color > 3u) {
                    current_color = 1u;
                }
                ui_update_palette_bar(current_color);
            }

            /* Cursor movement (stay out of UI rows) */
            if (input_pressed(BTN_LEFT)) {
                if (cursor_x > 0) cursor_x--;
            }
            if (input_pressed(BTN_RIGHT)) {
                if (cursor_x < (CANVAS_WIDTH - 1u)) cursor_x++;
            }
            if (input_pressed(BTN_UP)) {
                if (cursor_y > UI_FIRST_DRAW_ROW) cursor_y--;
            }
            if (input_pressed(BTN_DOWN)) {
                if (cursor_y < (CANVAS_HEIGHT - 1u)) cursor_y++;
            }

            /* Tool action on A pres */
            if (input_pressed(BTN_A)) {
                switch (s_current_tool) {
                    case TOOL_BRUSH:
                        canvas_set_pixel(cursor_x, cursor_y, current_color);
                        canvas_render_tile(cursor_x, cursor_y);
                        // s_full_redraw_needed = 1;    Dont redraw full screen, unnecessary for simple tools
                        break;

                    case TOOL_ERASER:
                        canvas_set_pixel(cursor_x, cursor_y, 0);
                        canvas_render_tile(cursor_x, cursor_y);
                        // s_full_redraw_needed = 1;    Dont redraw full screen, unnecessary for simple tools
                        break;

                    case TOOL_LINE:
                        if (!s_drag_active) {
                            s_drag_active = 1;
                            s_drag_start_x = cursor_x;
                            s_drag_start_y = cursor_y;
                        } else {
                            draw_line(s_drag_start_x, s_drag_start_y,
                                      cursor_x, cursor_y, current_color);
                            s_drag_active = 0;
                            s_full_redraw_needed = 1;
                        }
                        break;

                    case TOOL_RECT:
                        if (!s_drag_active) {
                            s_drag_active = 1;
                            s_drag_start_x = cursor_x;
                            s_drag_start_y = cursor_y;
                        } else {
                            draw_rect(s_drag_start_x, s_drag_start_y,
                                      cursor_x, cursor_y, current_color);
                            s_drag_active = 0;
                            s_full_redraw_needed = 1;
                        }
                        break;

                    case TOOL_FILL:
                        tool_flood_fill(cursor_x, cursor_y, current_color);
                        s_drag_active = 0;
                        s_full_redraw_needed = 1;
                        break;

                    default:
                        break;
                }
            }

            /* B cancels a pending line/rect "first click" */
            if (input_pressed(BTN_B)) {
                s_drag_active = 0;
            }
        }

        /* If a big tool ran, redraw the whole canvas with rendering OFF. */
        if (s_full_redraw_needed) {
            uint8_t oldMask = PPUMASK;

            PPUMASK = 0x00; /*rendering off*/

            /* Write entire nametable from g_canvas. */
            canvas_render_full();

            /* Reset scroll after VRAM writes. */
            PPUSCROLL = 0;
            PPUSCROLL = 0;
            PPUMASK = 0x18; /* expliicty call sprites*/
            /* Turn rendering back on. */
            // PPUMASK = oldMask;

            s_full_redraw_needed = 0;
        } else {
            /* Just keep scroll anchored this frame. */
            PPUSCROLL = 0;
            PPUSCROLL = 0;
        }

        /* Update cursor sprite position */
        sprite_x = (uint8_t)(cursor_x * 8u);
        sprite_y = (uint8_t)(cursor_y * 8u);
        ppu_draw_cursor_sprite(3, sprite_x, sprite_y);
    }
}
