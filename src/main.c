#include <stdint.h>
#include "ppu.h"
#include "input.h"
#include "canvas.h"

/* Background palette:
 * palette 0: universal + three colors used by tiles 1-3.
 */
static const uint8_t s_bg_palette[16] = {
    0x0F, 0x01, 0x11, 0x21,   /* palette 0: background + 3 blues */
    0x0F, 0x06, 0x16, 0x26,   /* palette 1 (unused) */
    0x0F, 0x09, 0x19, 0x29,   /* palette 2 (unused) */
    0x0F, 0x0A, 0x1A, 0x2A    /* palette 3 (unused) */
};

void main(void) {
    uint8_t cursor_x;
    uint8_t cursor_y;
    uint8_t current_color;

    /* Start cursor in middle of canvas */
    cursor_x = CANVAS_WIDTH  / 2;
    cursor_y = CANVAS_HEIGHT / 2;

    /* Default brush color = 1 (uses tile 1) */
    current_color = 1;

    /* 1) Basic PPU init (rendering OFF). */
    ppu_init();

    /* 2) While rendering is off, set palette and initial canvas. */
    ppu_wait_vblank();
    ppu_load_bg_palette(s_bg_palette, 16);

    /* Start with a blank canvas (color 0). */
    canvas_clear(0);
    canvas_render_full();

    /* 3) Clear sprite memory and place initial cursor sprite. */
    ppu_clear_oam();

    {
        uint8_t sprite_x = (uint8_t)(cursor_x * 8u);
        uint8_t sprite_y = (uint8_t)(cursor_y * 8u + 1u);
        /* Use tile 3 (solid color #3) as cursor for now. */
        ppu_draw_cursor_sprite(3, sprite_x, sprite_y);
    }

    /* 4) Turn rendering ON: background + sprites, nametable 0, no NMI. */
    PPUCTRL = 0x00;
    PPUMASK = 0x18;   /* bit3=BG, bit4=sprites */

    /* 5) Main loop */
    for (;;) {
        uint8_t sprite_x;
        uint8_t sprite_y;

        /* 1) Wait for vblank: safe time for VRAM/OAM */
        ppu_wait_vblank();

        /* 2) Poll controller */
        input_update();

        /* 3) Cursor movement (same as before) */
        if (input_pressed(BTN_LEFT)) {
            if (cursor_x > 0) cursor_x--;
        }
        if (input_pressed(BTN_RIGHT)) {
            if (cursor_x < (CANVAS_WIDTH - 1u)) cursor_x++;
        }
        if (input_pressed(BTN_UP)) {
            if (cursor_y > 0) cursor_y--;
        }
        if (input_pressed(BTN_DOWN)) {
            if (cursor_y < (CANVAS_HEIGHT - 1u)) cursor_y++;
        }

        /* 4) Brush: paint with A (update RAM + ONE tile in VRAM) */
        if (input_pressed(BTN_A)) {
            canvas_set_pixel(cursor_x, cursor_y, current_color);
            canvas_render_tile(cursor_x, cursor_y);
        }

        /* 5) Reset scroll so background always starts at (0,0) */
        PPUSCROLL = 0;
        PPUSCROLL = 0;

        /* 6) Update cursor sprite position */
        sprite_x = (uint8_t)(cursor_x * 8u);
        sprite_y = (uint8_t)(cursor_y * 8u + 1u);
        ppu_draw_cursor_sprite(3, sprite_x, sprite_y);
    }

}
