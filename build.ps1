#Powershell scripts specifically for Windows
#Run script in Windows Powershell for .nes file for emulator
$CC65BIN = "C:\cc65\bin"
$env:PATH = "$CC65BIN;$env:PATH"

$SRC_DIR   = "src"
$ASM_DIR   = "asm"
$CFG_DIR   = "cfg"
$BUILD_DIR = "build"
$BIN_DIR   = "bin"

New-Item -ItemType Directory -Force -Path $BUILD_DIR, $BIN_DIR | Out-Null

# C -> .s
cc65 -Oirs -t nes --add-source -I include `
    -o "$BUILD_DIR/main.s"   "$SRC_DIR/main.c"
cc65 -Oirs -t nes --add-source -I include `
    -o "$BUILD_DIR/ppu.s"    "$SRC_DIR/ppu.c"
cc65 -Oirs -t nes --add-source -I include `
    -o "$BUILD_DIR/input.s"  "$SRC_DIR/input.c"
cc65 -Oirs -t nes --add-source -I include `
    -o "$BUILD_DIR/canvas.s" "$SRC_DIR/canvas.c"

# .s -> .o
ca65 -t nes -o "$BUILD_DIR/main.o"   "$BUILD_DIR/main.s"
ca65 -t nes -o "$BUILD_DIR/ppu.o"    "$BUILD_DIR/ppu.s"
ca65 -t nes -o "$BUILD_DIR/input.o"  "$BUILD_DIR/input.s"
ca65 -t nes -o "$BUILD_DIR/canvas.o" "$BUILD_DIR/canvas.s"

# asm -> .o
ca65 -t nes -o "$BUILD_DIR/crt0.o"  "$ASM_DIR/crt0.s"
ca65 -t nes -o "$BUILD_DIR/tiles.o" "$ASM_DIR/tiles.s"

# Link
ld65 -C "$CFG_DIR/nes.cfg" `
     -o "$BIN_DIR/game.nes" `
     "$BUILD_DIR/crt0.o" `
     "$BUILD_DIR/tiles.o" `
     "$BUILD_DIR/main.o" `
     "$BUILD_DIR/ppu.o" `
     "$BUILD_DIR/input.o" `
     "$BUILD_DIR/canvas.o" `
     nes.lib

Write-Host "Built bin/game.nes"
