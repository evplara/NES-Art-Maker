# Adjust CC65 if not on PATH, e.g.:
# CC65_HOME := /usr/local/cc65
# PATH := $(CC65_HOME)/bin:$(PATH)

OUT = game.nes

all: $(OUT)

$(OUT): crt0.s main.c nes.cfg
	# Compile + assemble + link in one go with cl65
	cl65 -t none -o $(OUT) -C nes.cfg crt0.s main.c

clean:
	rm -f *.o *.s $(OUT)
