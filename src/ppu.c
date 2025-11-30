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
    /* Sync with PPU so we don't write during startup garbage. */
    ppu_wait_vblank();

    /* Reset scroll */
    PPUSCROLL = 0;
    PPUSCROLL = 0;

    /* Base nametable 0, no NMI (we're polling vblank manually). */
    PPUCTRL = 0x00;

    /* Show background, hide sprites for now. */
    PPUMASK = 0x08;
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
