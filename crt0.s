; crt0.s — minimal startup for cc65 + NES (NROM-256, CHR-RAM)
.export _nmi, _reset, _irq
.import _main

.segment "HEADER"
; iNES header (16 bytes)
; "NES" 1A, PRG=2 x16KB, CHR=0 (CHR-RAM), flags6=0 (H mirroring, mapper 0)
  .byte $4E,$45,$53,$1A
  .byte $02   ; 2 PRG banks = 32KB
  .byte $00   ; 0 CHR banks -> CHR-RAM
  .byte $00   ; flags6: mapper 0, horizontal mirroring
  .byte $00   ; flags7
  .byte $00,$00,$00,$00,$00,$00,$00,$00

; PPU/APU registers
PPUCTRL   = $2000
PPUMASK   = $2001
PPUSTATUS = $2002
APUFC     = $4017
APUDMC    = $4010

.segment "CODE"
_reset:
  sei
  cld

  ; Disable APU frame IRQs
  ldx #$40
  stx APUFC

  ; Init stack
  ldx #$FF
  txs

  ; Disable rendering & DMC IRQ
  ldx #$00
  stx PPUCTRL
  stx PPUMASK
  stx APUDMC

  ; Wait for vblank twice (stable PPU)
  bit PPUSTATUS
@v1:
  bit PPUSTATUS
  bpl @v1
@v2:
  bit PPUSTATUS
  bpl @v2

  ; Clear RAM $0000-$07FF
  ldx #$00
  txa
@clr:
  sta $0000,x
  sta $0100,x
  sta $0200,x
  sta $0300,x
  sta $0400,x
  sta $0500,x
  sta $0600,x
  sta $0700,x
  inx
  bne @clr

  ; Jump to C main()
  jsr _main

@hang: jmp @hang

_nmi:
  ; You can call a C function here if you export/import it.
  rti

_irq:
  rti

.segment "VECTORS"
  .addr _nmi
  .addr _reset
  .addr _irq
