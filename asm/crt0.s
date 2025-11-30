; crt0.s — minimal cc65 NES startup (NROM-256, CHR-ROM)
;
; - Uses standard cc65 NES memory layout from nes.cfg
; - Initializes C runtime (BSS/DATA, constructors)
; - Does minimal NES hardware init, then jumps to C main()
;
; Assemble with:   ca65 -t nes crt0.s

        .export _nmi, _reset, _irq
        .export __STARTUP__ : absolute = 1

        .import _main
        .import __RAM_START__, __RAM_SIZE__
        .import zerobss, copydata, initlib, donelib

        .importzp c_sp

PPUCTRL   = $2000
PPUMASK   = $2001
PPUSTATUS = $2002
APUFC     = $4017
DMCCTRL   = $4010

;--------------------------------------------------------------
; iNES header (32KB PRG, 8KB CHR, mapper 0, H-mirror)
;--------------------------------------------------------------
.segment "HEADER"
        .byte $4E,$45,$53,$1A   ; "NES"+$1A
        .byte $02               ; 2 x 16KB PRG = 32KB
        .byte $01               ; 1 x 8KB CHR
        .byte $00               ; mapper 0, horizontal mirroring
        .byte $00               ; no battery, no trainer, mapper high nibble = 0
        .byte $00,$00,$00,$00,$00,$00,$00,$00

;--------------------------------------------------------------
; Reset / startup
;--------------------------------------------------------------
.segment "STARTUP"

_reset:
        sei                     ; disable IRQ
        cld                     ; clear decimal mode

        ; Disable APU frame IRQ and DMC IRQ
        lda #$40
        sta APUFC
        lda #$00
        sta DMCCTRL

        ; Initialize 6502 hardware stack at $01FF
        ldx #$FF
        txs

        ; Turn off NMI and rendering for now
        lda #$00
        sta PPUCTRL
        sta PPUMASK

        ; Wait for first vblank so PPU is ready
@wait1:
        bit PPUSTATUS           ; clear vblank flag
@vblank1:
        bit PPUSTATUS
        bpl @vblank1            ; loop until vblank flag set

        ; Set C argument stack pointer at top of RAM ($6000-$7FFF)
        lda #<(__RAM_START__ + __RAM_SIZE__)
        sta c_sp
        lda #>(__RAM_START__ + __RAM_SIZE__)
        sta c_sp+1

        ; C runtime: clear BSS, copy DATA, run constructors
        jsr zerobss
        jsr copydata
        jsr initlib

        ; Call C main()
        jsr _main

_exit:
        jsr donelib             ; run destructors (if any)
@halt:
        jmp @halt               ; if main returns, just spin

;--------------------------------------------------------------
; Interrupt stubs (we just ignore IRQ/NMI for now)
;--------------------------------------------------------------
_nmi:
_irq:
        rti

;--------------------------------------------------------------
; Vector table (mapped by VECTORS segment in nes.cfg)
;--------------------------------------------------------------
.segment "VECTORS"
        .addr _nmi
        .addr _reset
        .addr _irq
