OUT = game.nes

CC65 ?= C:/cc65
LIBDIR := $(CC65)/lib

all: $(OUT)

main.s: main.c
	cc65 -Oirs -t nes main.c -o main.s

main.o: main.s
	ca65 main.s -o main.o

crt0.o: crt0.s
	ca65 crt0.s -o crt0.o

$(OUT): crt0.o main.o nes.cfg
	cl65 -t nes -C nes.cfg -o $(OUT) crt0.o main.o

clean:
	-del /Q main.s *.o $(OUT) 2> NUL
