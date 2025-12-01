#include <stdint.h>
#include "ppu.h"
#include "input.h"
#include "canvas.h"
#define UI_PALETTE_ROW      0
#define UI_HIGHLIGHT_ROW    1
#define UI_FIRST_DRAW_ROW   2

/* Tools*/
typedef enum {
    TOOL_BRUSH = 0,
    TOOL_ERASER,
    TOOL_LINE,
    TOOL_RECT,
    TOOL_FILL,
    TOOL_COUNT
} Tool;

#define FILL_QUEUE_MAX (CANVAS_WIDTH * CANVAS_HEIGHT)

typedef struct {
    uint8_t x;
    uint8_t y;
} FillNode;

static FillNode s_fill_queue[FILL_QUEUE_MAX];

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
    0x0F, 0x06, 0x16, 0x26,   /* palette 1 (unused) */
    0x0F, 0x09, 0x19, 0x29,   /* palette 2 (unused) */
    0x0F, 0x0A, 0x1A, 0x2A    /* palette 3 (unused) */
};

/* Background palette */
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
   Different color patterns for each tool (use less CHR tiles, no need for text sprites):
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

void main(void) {
    uint8_t cursor_x;
    uint8_t cursor_y;
    uint8_t current_color;

    /* Start cursor in middle of canvas */
    cursor_x = CANVAS_WIDTH  / 2;
    cursor_y = (CANVAS_HEIGHT + UI_FIRST_DRAW_ROW) / 2;

    /* Default brush color = 1 (uses tile 1) */
    current_color = 1;

    /*  Basic PPU init (rendering OFF). */
    ppu_init();

    /* While rendering is off, set palette and initial canvas. */
    ppu_wait_vblank();
    ppu_load_bg_palette(s_bg_palette, 16);

    /* Start with a blank canvas (color 0). */
    canvas_clear(0);
    /* Draw initial palette bar into the canvas (current_color = 1) */
    current_color = 1;
    ui_update_palette_bar(current_color);

    canvas_render_full();

    /* Clear sprite memory and place initial cursor sprite. */
    ppu_clear_oam();

    {
        uint8_t sprite_x = (uint8_t)(cursor_x * 8u);
        uint8_t sprite_y = (uint8_t)(cursor_y * 8u + 1u);
        /* Use tile 3 (solid color #3) as cursor for now. */
        ppu_draw_cursor_sprite(3, sprite_x, sprite_y);
    }

    /* Turn rendering ON: background + sprites, nametable 0, no NMI. */
    PPUCTRL = 0x00;
    PPUMASK = 0x18;   /* bit3=BG, bit4=sprites */

    /*) Main loop */
    for (;;) {
        uint8_t sprite_x;
        uint8_t sprite_y;

        /* Wait for vblank: safe time for VRAM/OAM */
        ppu_wait_vblank();

        /* Poll controller */
        input_update();

        /* Cursor movement (same as before) */
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

        /* Color Selection with Select*/
        if (input_pressed(BTN_SELECT)) {
            current_color++;
            if (current_color > 3u) {
                current_color = 1u;   /* wrap around */
            }
            ui_update_palette_bar(current_color);
        }

        /* Brush: paint with A (update RAM + ONE tile in VRAM) */
        if (input_pressed(BTN_A)) {
            canvas_set_pixel(cursor_x, cursor_y, current_color);
            canvas_render_tile(cursor_x, cursor_y);
        }

        /*Eraser with B, paints color 0*/
        if (input_pressed(BTN_B)) {
            canvas_set_pixel(cursor_x, cursor_y, 0);
            canvas_render_tile(cursor_x, cursor_y);
        }
        /* Reset scroll so background always starts at (0,0) */
        PPUSCROLL = 0;
        PPUSCROLL = 0;

        /* Update cursor sprite position */
        sprite_x = (uint8_t)(cursor_x * 8u);
        sprite_y = (uint8_t)(cursor_y * 8u + 1u);
        ppu_draw_cursor_sprite(3, sprite_x, sprite_y);
    }

}
