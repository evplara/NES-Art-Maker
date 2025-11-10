// main.c — cc65-friendly (C89: declarations before statements)

#include <stdint.h>

#define PPUCTRL   (*(volatile uint8_t*)0x2000)
#define PPUMASK   (*(volatile uint8_t*)0x2001)
#define PPUSTATUS (*(volatile uint8_t*)0x2002)
#define OAMDMA    (*(volatile uint8_t*)0x4014)
#define PPUSCROLL (*(volatile uint8_t*)0x2005)
#define PPUADDR   (*(volatile uint8_t*)0x2006)
#define PPUDATA   (*(volatile uint8_t*)0x2007)

static void wait_vblank(void) {
    /* C89-compliant: no new declarations after statements here */
    /* ensure we're OUT of vblank */
    while (PPUSTATUS & 0x80) { }
    /* wait until IN vblank */
    while (!(PPUSTATUS & 0x80)) { }
}

static void ppu_write16(uint16_t addr, uint8_t value) {
    PPUADDR = (uint8_t)(addr >> 8);
    PPUADDR = (uint8_t)(addr & 0xFF);
    PPUDATA = value;
}

void main(void) {
    uint8_t color;  /* declare at top (C89 rule) */

    /* Basic PPU init */
    PPUSCROLL = 0;
    PPUSCROLL = 0;

    /* Enable NMI (bit 7) and base nametable 0 */
    PPUCTRL = 0x80;

    /* Show background/sprites */
    PPUMASK = 0x1E;

    color = 0;  /* initialize after declarations */

    for (;;) {
        wait_vblank();

        /* universal background color at $3F00 */
        ppu_write16(0x3F00, color);

        color++;
        if (color >= 64) color = 0;

        /* If you add delays, declare loop vars at block top per C89 */
        /* {
             uint16_t d;
             for (d = 0; d < 3000; ++d) { }
           } */
    }
}
