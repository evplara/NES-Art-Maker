; tiles.s - Simple CHR-ROM tiles for solid color blocks
; Each NES background tile = 16 bytes:
;   - first 8 bytes: bitplane 0 (low bits)
;   - next 8 bytes: bitplane 1 (high bits)
;
; We define 4 tiles:
;   0: all 00 (background color)
;   1: all 01 (palette color #1)
;   2: all 10 (palette color #2)
;   3: all 11 (palette color #3)

.segment "CHARS"

; Tile 0 - all background (color index 0)
.repeat 16
    .byte %00000000
.endrepeat

; Tile 1 - solid color index 1 (01)
.repeat 8
    .byte %11111111   ; plane 0 bits = 1
.endrepeat
.repeat 8
    .byte %00000000   ; plane 1 bits = 0
.endrepeat

; Tile 2 - solid color index 2 (10)
.repeat 8
    .byte %00000000   ; plane 0 bits = 0
.endrepeat
.repeat 8
    .byte %11111111   ; plane 1 bits = 1
.endrepeat

; Tile 3 - solid color index 3 (11)
.repeat 8
    .byte %11111111   ; plane 0 bits = 1
.endrepeat
.repeat 8
    .byte %11111111   ; plane 1 bits = 1
.endrepeat

; ld65 will fill the rest of the 8KB CHR bank with zeros.
