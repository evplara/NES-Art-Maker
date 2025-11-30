/* ppu.c - Implementation of small PPU helper API */

#include <stdint.h>
#include "ppu.h"

/* Poll PPUSTATUS bit 7 (vblank) until a new vblank begins. */
void ppu_wait_vblank(void) {
    /* Make sure we are out of the old vblank first. */
    while (PPUSTATUS & 0x80) {
        /* spin */
    }
    /* Now wait for the next vblank to start. */
    while (!(PPUSTATUS & 0x80)) {
        /* spin */
    }
}

/* Minimal PPU setup: background only, no NMI, scroll at (0,0). */
void ppu_init(void) {
    /* Sync with PPU first */
    ppu_wait_vblank();

    /* Turn rendering off while we set things up. */
    PPUCTRL = 0x00;
    PPUMASK = 0x00;

    /* Reset scroll */
    PPUSCROLL = 0;
    PPUSCROLL = 0;
}


void ppu_write_vram(uint16_t addr, uint8_t value) {
    PPUADDR = (uint8_t)(addr >> 8);
    PPUADDR = (uint8_t)(addr & 0xFF);
    PPUDATA = value;
}

void ppu_fill_vram(uint16_t addr, uint16_t len, uint8_t value) {
    uint16_t i;

    PPUADDR = (uint8_t)(addr >> 8);
    PPUADDR = (uint8_t)(addr & 0xFF);

    for (i = 0; i < len; ++i) {
        PPUDATA = value;
    }
}

void ppu_load_bg_palette(const uint8_t* data, uint8_t count) {
    uint8_t i;

    /* After ppu_wait_vblank(), PPUSTATUS has been read so the address latch
       is in a defined state and we can safely write PPUADDR. */
    PPUADDR = 0x3F;
    PPUADDR = 0x00;

    for (i = 0; i < count; ++i) {
        PPUDATA = data[i];
    }
}

void ppu_clear_oam(void) {
    uint8_t i;

    /* Clear all 64 sprites: put them off-screen. */
    OAMADDR = 0;
    for (i = 0; i < 64u; ++i) {
        OAMDATA = 0xFF;  /* Y = 255 -> hidden */
        OAMDATA = 0x00;  /* tile */
        OAMDATA = 0x00;  /* attributes */
        OAMDATA = 0x00;  /* X */
    }
}

/* Draw cursor as sprite #0 using given tile and screen coordinates. */
void ppu_draw_cursor_sprite(uint8_t tile, uint8_t x, uint8_t y) {
    /* NES sprites use Y coordinate as (top - 1), but we’ll pass in
       the value we want and just use it directly for now. */
    OAMADDR = 0;      /* sprite 0 */
    OAMDATA = y;      /* Y position */
    OAMDATA = tile;   /* tile index */
    OAMDATA = 0x00;   /* attributes: palette 0, no flip */
    OAMDATA = x;      /* X position */
}
