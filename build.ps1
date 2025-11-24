cc65 -Oirs -t nes --add-source main.c

ca65 -t nes crt0.s
ca65 -t nes main.s

ld65 -C nes.cfg -o game.nes crt0.o main.o nes.lib

Write-Host "Built game.nes"
