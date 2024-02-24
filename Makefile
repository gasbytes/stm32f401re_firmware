# Compiler
CC = arm-none-eabi-gcc

# Directories
SRC_DIR = src
OBJ_DIR = obj
INIT_DIR = init
OUT_DIR = out
INC_DIR = inc

# Files
SRC := $(wildcard $(SRC_DIR)/*.c)
SRC += $(wildcard $(INIT_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(SRC_DIR)/$(OBJ_DIR)/%.o, $(SRC))
OBJ := $(patsubst $(INIT_DIR)/%.c, $(SRC_DIR)/$(OBJ_DIR)/%.o, $(OBJ))
LD := $(wildcard $(INIT_DIR)/*.ld)

# FLAGS
MARCH = cortex-m4
CFLAGS = -g -Wall -mcpu=$(MARCH) -mthumb -mfloat-abi=soft -ffreestanding -nostartfiles -I$(INC_DIR)
LFLAGS = -nostdlib -T $(LD) -Wl,-Map=$(OUT_DIR)/out.map

# Targets
TARGET = $(OUT_DIR)/out.elf

all: $(OBJ) $(TARGET) bin

$(SRC_DIR)/$(OBJ_DIR)/%.o : $(SRC_DIR)/%.c | mkobj
	$(CC) $(CFLAGS) -c -o $@ $^

$(SRC_DIR)/$(OBJ_DIR)/%.o : $(INIT_DIR)/%.c | mkobj
	$(CC) $(CFLAGS) -c -o $@ $^

$(TARGET) : $(OBJ) | mkdeb
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^

mkobj:
	mkdir -p $(SRC_DIR)/$(OBJ_DIR)

mkdeb:
	mkdir -p $(OUT_DIR)

bin:
	arm-none-eabi-objcopy -O binary $(OUT_DIR)/out.elf $(OUT_DIR)/out.bin

flash:
	st-flash --reset write $(OUT_DIR)/out.bin 0x8000000

clean:
	rm -rf out/
