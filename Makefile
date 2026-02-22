# Toolchain
PREFIX = arm-none-eabi-
CC = $(PREFIX)gcc
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
SIZE = $(PREFIX)size
GDB = $(PREFIX)gdb

# Target name
TARGET = motors_l298n

# Directories
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj

# Source files
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/system.c \
       $(SRC_DIR)/gpio.c \
       $(SRC_DIR)/pwm.c \
       $(SRC_DIR)/motor.c \
       startup.s

# Object files
OBJS = $(SRCS:%.c=$(OBJ_DIR)/%.o)
OBJS := $(OBJS:%.s=$(OBJ_DIR)/%.o)

# Compiler flags
CFLAGS = -mcpu=cortex-m3 -mthumb -mfloat-abi=soft
CFLAGS += -O0 -g
CFLAGS += -Wall -Wextra -Wpedantic
CFLAGS += -fno-common -ffunction-sections -fdata-sections
CFLAGS += -I$(INC_DIR)
CFLAGS += -DDEBUG -DSTM32F103x8
CFLAGS += -nostdlib
ASFLAGS = -mcpu=cortex-m3 -mthumb

# Linker flags
LDFLAGS = -T stm32f103c8.ld -nostartfiles
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -nostdlib
LDFLAGS += -lgcc
LDFLAGS += -Wl,-Map=$(TARGET).map

# Rules
all: $(OBJ_DIR) $(TARGET).bin

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)/src

$(OBJ_DIR)/%.o: %.c
	@echo "  CC    $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.s
	@echo "  AS    $<"
	@$(CC) $(ASFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	@echo "  LD    $@"
	@$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lgcc
	@echo "\nSize report:"
	@$(SIZE) $@

$(TARGET).bin: $(TARGET).elf
	@echo "  OBJCOPY $@"
	@$(OBJCOPY) -O binary $< $@
	@$(OBJDUMP) -D $< > $(TARGET).dis
	@echo "\nBuild completed: $(TARGET).bin"

# Flash with ST-Link
flash: $(TARGET).bin
	st-flash --reset write $(TARGET).bin 0x08000000

# Flash with OpenOCD
flash-openocd: $(TARGET).elf
	openocd -f interface/stlink-v2.cfg -f target/stm32f1x.cfg \
	-c "program $(TARGET).elf verify reset exit"

# Debug with GDB
debug: $(TARGET).elf
	$(GDB) -ex "target remote :3333" -ex "monitor reset halt" \
	-ex "load" -ex "monitor reset halt" $(TARGET).elf

# Clean
clean:
	rm -rf $(OBJ_DIR) *.elf *.bin *.dis *.map

# Directory creation
dirs:
	mkdir -p $(SRC_DIR) $(INC_DIR) $(OBJ_DIR)/src

.PHONY: all clean flash flash-openocd debug dirs