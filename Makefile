CC65BIN = C:/cc65/bin

CC  = $(CC65BIN)/cc65
CA  = $(CC65BIN)/ca65
LD  = $(CC65BIN)/ld65

SRC_DIR   = src
INC_DIR   = include
ASM_DIR   = asm
CFG_DIR   = cfg
BUILD_DIR = build
BIN_DIR   = bin

CFLAGS  = -Oirs -t nes --add-source -I $(INC_DIR)
AFLAGS  = -t nes
LDFLAGS = -C $(CFG_DIR)/nes.cfg

C_SOURCES = \
	$(SRC_DIR)/main.c \
	$(SRC_DIR)/ppu.c  \
	$(SRC_DIR)/input.c \
	$(SRC_DIR)/canvas.c

ASM_SOURCES = \
	$(ASM_DIR)/crt0.s \
	$(ASM_DIR)/tiles.s

# Derived names in build directory
C_ASM = $(C_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.s)
OBJS  = $(C_SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o) \
        $(ASM_SOURCES:$(ASM_DIR)/%.s=$(BUILD_DIR)/%.o)

# Default target
all: $(BIN_DIR)/game.nes

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

$(BUILD_DIR)/%.s: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: $(BUILD_DIR)/%.s
	$(CA) $(AFLAGS) -o $@ $<

$(BUILD_DIR)/%.o: $(ASM_DIR)/%.s | $(BUILD_DIR)
	$(CA) $(AFLAGS) -o $@ $<

$(BIN_DIR)/game.nes: $(OBJS) $(CFG_DIR)/nes.cfg | $(BIN_DIR)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) nes.lib

clean:
	-del /Q $(BUILD_DIR)\*.o $(BUILD_DIR)\*.s 2> NUL
	-del /Q $(BIN_DIR)\game.nes 2> NUL
