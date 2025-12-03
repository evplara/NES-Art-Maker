/* ppu.h - Small helper API around NES PPU registers */

#ifndef PPU_H
#define PPU_H

#include <stdint.h>

/* PPU registers (memory-mapped) */
#define PPUCTRL   (*(volatile uint8_t*)0x2000)
#define PPUMASK   (*(volatile uint8_t*)0x2001)
#define PPUSTATUS (*(volatile uint8_t*)0x2002)
#define OAMADDR   (*(volatile uint8_t*)0x2003)
#define OAMDATA   (*(volatile uint8_t*)0x2004)
#define PPUSCROLL (*(volatile uint8_t*)0x2005)
#define PPUADDR   (*(volatile uint8_t*)0x2006)
#define PPUDATA   (*(volatile uint8_t*)0x2007)

/* Wait for the start of vblank (safe window for VRAM writes). */
void ppu_wait_vblank(void);

/* Basic PPU init: scroll (0,0), background on, sprites off, NMI off. */
void ppu_init(void);

/* Write one byte to VRAM at 16-bit address (call during vblank). */
void ppu_write_vram(uint16_t addr, uint8_t value);

/* Fill VRAM region [addr, addr+len) with value (during vblank). */
void ppu_fill_vram(uint16_t addr, uint16_t len, uint8_t value);

/* Load background palette bytes to $3F00.. (count bytes). */
void ppu_load_bg_palette(const uint8_t* data, uint8_t count);
/* For sprite cursor*/
void ppu_clear_oam(void);
void ppu_draw_cursor_sprite(uint8_t tile, uint8_t x, uint8_t y);

#endif
