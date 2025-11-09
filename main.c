// main.c — buildable with cc65 (no extra libs)
// Shows a solid background and cycles color each frame.

#include <stdint.h>

#define PPUCTRL   (*(volatile uint8_t*)0x2000)
#define PPUMASK   (*(volatile uint8_t*)0x2001)
#define PPUSTATUS (*(volatile uint8_t*)0x2002)
#define OAMDMA    (*(volatile uint8_t*)0x4014)
#define PPUSCROLL (*(volatile uint8_t*)0x2005)
#define PPUADDR   (*(volatile uint8_t*)0x2006)
#define PPUDATA   (*(volatile uint8_t*)0x2007)

static void wait_vblank(void) {
    // Wait for vblank start: bit 7 of PPUSTATUS becomes 1 at vblank
    // First read clears the latch, then spin until bit 7 is set.
    while (PPUSTATUS & 0x80) { }   // ensure we're OUT of vblank
    while (!(PPUSTATUS & 0x80)) { } // wait until IN vblank
}

static void ppu_write16(uint16_t addr, uint8_t value) {
    PPUADDR = (uint8_t)(addr >> 8);
    PPUADDR = (uint8_t)(addr & 0xFF);
    PPUDATA = value;
}

void main(void) {
    // Basic PPU init: scrolling zero
    PPUSCROLL = 0;
    PPUSCROLL = 0;

    // Enable NMI (bit 7) and set base nametable 0.
    PPUCTRL = 0x80;

    // Turn ON background (bit5) and sprites (bit6) if desired.
    // Even with no tiles uploaded, you'll see the universal background color.
    PPUMASK = 0x1E; // show bg/sprites, no clipping

    uint8_t color = 0x00; // NES palette index (0-63 usable values)
    for (;;) {
        wait_vblank();

        // Write universal background color at $3F00
        ppu_write16(0x3F00, color);

        // simple color cycle
        color++;
        if (color >= 64) color = 0;

        // Optionally slow down the cycle:
        // for (volatile uint16_t d = 0; d < 3000; ++d) { }
    }
}
