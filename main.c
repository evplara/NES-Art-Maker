// main.c — NES background color cycler 

#include <stdint.h>

/* PPU / APU registers (memory-mapped I/O) */
#define PPUCTRL   (*(volatile uint8_t*)0x2000)
#define PPUMASK   (*(volatile uint8_t*)0x2001)
#define PPUSTATUS (*(volatile uint8_t*)0x2002)
#define OAMADDR   (*(volatile uint8_t*)0x2003)
#define OAMDATA   (*(volatile uint8_t*)0x2004)
#define PPUSCROLL (*(volatile uint8_t*)0x2005)
#define PPUADDR   (*(volatile uint8_t*)0x2006)
#define PPUDATA   (*(volatile uint8_t*)0x2007)

/* Wait for the start of vblank.
   Pattern from cc65 docs: poll PPUSTATUS bit 7. */
static void wait_vblank(void) {
    /* Ensure we're OUT of vblank first */
    while (PPUSTATUS & 0x80) {
        /* spin */
    }
    /* Now wait until vblank starts */
    while (!(PPUSTATUS & 0x80)) {
        /* spin */
    }
}

/* Write a single byte to PPU VRAM at a 16-bit address.
   Must be called only just after a PPUSTATUS read (wait_vblank). */
static void ppu_write(uint16_t addr, uint8_t value) {
    PPUADDR = (uint8_t)(addr >> 8);     /* high byte */
    PPUADDR = (uint8_t)(addr & 0xFF);   /* low byte  */
    PPUDATA = value;
}

/* Fill nametable 0 ($2000) with tile 0 and attribute table with 0.
   This makes the whole background use palette 0, tile 0. */
static void ppu_clear_nametable(void) {
    uint16_t i;

    /* Nametable 0: $2000–$23BF (32x30 = 960 bytes) */
    PPUADDR = 0x20;
    PPUADDR = 0x00;
    for (i = 0; i < 32u * 30u; ++i) {
        PPUDATA = 0;
    }

    /* Attribute table at $23C0–$23FF (64 bytes) */
    PPUADDR = 0x23;
    PPUADDR = 0xC0;
    for (i = 0; i < 64u; ++i) {
        PPUDATA = 0;
    }
}

/* Initialize palette: set universal background color and some simple colors. */
static void ppu_init_palette(uint8_t bg_color) {
    uint8_t i;

    /* Start at $3F00, background palettes */
    PPUADDR = 0x3F;
    PPUADDR = 0x00;

    /* Universal background color (backdrop) */
    PPUDATA = bg_color;

    /* Three entries of background palette 0: arbitrary visible colors */
    PPUDATA = 0x01;  /* dark blue */
    PPUDATA = 0x11;  /* medium blue */
    PPUDATA = 0x21;  /* light blue */

    /* Remaining 3 background palettes (12 bytes) — just fill with 0 for now */
    for (i = 0; i < 12u; ++i) {
        PPUDATA = 0x00;
    }
}

/* Entry point called by crt0.s */
void main(void) {
    uint8_t color;

    color = 0;  /* start from color 0 */

    /* Sync with PPU */
    wait_vblank();

    /* Clear nametable and attributes so the whole screen uses palette 0 */
    ppu_clear_nametable();

    /* Initialize palette with some starting background color (e.g. 0x0F) */
    ppu_init_palette(0x0F);

    /* Enable background rendering (no NMI, just polling) */
    PPUCTRL = 0x00;    /* base nametable 0, no NMI */
    PPUMASK = 0x08;    /* show background, no sprites */

    /* Main loop: cycle universal background color at $3F00 */
    for (;;) {
        /* Wait for start of vblank so VRAM writes are safe */
        wait_vblank();

        /* Write new backdrop color into $3F00 */
        ppu_write(0x3F00, color);

        /* Cycle through 64 palette entries (0x00–0x3F) */
        ++color;
        if (color >= 0x40) {
            color = 0;
        }
    }
}
