OUT = game.nes

# Point this to your cc65 install directory if needed, e.g. C:/cc65
CC65 ?= C:/cc65
LIBDIR := $(CC65)/lib

all: $(OUT)

# 1) C -> ASM (use the NES target)
main.s: main.c
	cc65 -Oirs -t nes main.c -o main.s

# 2) ASM -> OBJ
main.o: main.s
	ca65 main.s -o main.o

crt0.o: crt0.s
	ca65 crt0.s -o crt0.o

# 3) Link objects with our cfg AND the NES library
# Driver handles libraries; easy mode
$(OUT): crt0.o main.o nes.cfg
	cl65 -t nes -C nes.cfg -o $(OUT) crt0.o main.o
# (you usually don't even need -L or -lnes with cl65 when -t nes is set)


clean:
	-del /Q main.s *.o $(OUT) 2> NUL
