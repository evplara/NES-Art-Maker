; crt0.s — minimal startup for cc65 + NES (NROM-256, CHR-RAM)
.export _nmi, _reset, _irq
.import _main

.segment "HEADER"
  .byte $4E,$45,$53,$1A   ; "NES"+$1A
  .byte $02               ; 2 x 16KB PRG
  .byte $00               ; 0 CHR (CHR-RAM)
  .byte $00               ; flags6: mapper 0, H mirror
  .byte $00               ; flags7
  .byte $00,$00,$00,$00,$00,$00,$00,$00

PPUCTRL   = $2000
PPUMASK   = $2001
PPUSTATUS = $2002
APUFC     = $4017
APUDMC    = $4010

.import _main
.export _nmi, _reset, _irq

.segment "STARTUP"
_reset:
  sei
  cld
  ldx #$40
  stx APUFC
  ldx #$FF
  txs
  ldx #$00
  stx PPUCTRL
  stx PPUMASK
  stx APUDMC
  bit PPUSTATUS
@v1: bit PPUSTATUS
  bpl @v1
@v2: bit PPUSTATUS
  bpl @v2

  ; clear $0000-$07FF
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

  jsr _main
@hang: jmp @hang

_nmi:
  rti
_irq:
  rti

.export __STARTUP__ := _reset


.segment "VECTORS"
  .addr _nmi
  .addr _reset
  .addr _irq

