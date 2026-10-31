# stm32-serial-cli
#
#   make test     build and run the unit tests on your PC
#   make host     build the CLI as a normal program for your PC terminal
#   make run      build and start the PC version
#   make stm32    build the firmware for the STM32C031C6 (needs arm-none-eabi-gcc)
#   make clean    delete the build folder

.DEFAULT_GOAL := help

CC        ?= cc
BUILD     := build
INCLUDES  := -Icore

CORE_SRC  := $(wildcard core/*.c)

# ---- PC build ---------------------------------------------------------------
HOST_CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -O2 -g $(INCLUDES)
HOST_SRC    := $(CORE_SRC) platform/host/hal_host.c platform/host/main_host.c
HOST_BIN    := $(BUILD)/host/serial-cli

# ---- Unit tests (with address and undefined-behaviour sanitizers) -----------
TEST_CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -g -O0 \
               -fsanitize=address,undefined -fno-omit-frame-pointer \
               $(INCLUDES) -Itests
TEST_SRC    := $(wildcard tests/test_*.c)
TEST_BINS   := $(patsubst tests/%.c,$(BUILD)/tests/%,$(TEST_SRC))

# ---- STM32C031C6 firmware ---------------------------------------------------
ARM_PREFIX  ?= arm-none-eabi-
ARM_CC      := $(ARM_PREFIX)gcc
ARM_CFLAGS  := -mcpu=cortex-m0plus -mthumb -Os -g -std=c11 -Wall -Wextra \
               -ffunction-sections -fdata-sections $(INCLUDES) -Iplatform/stm32c0
ARM_LDFLAGS := -T platform/stm32c0/linker.ld -nostartfiles \
               --specs=nano.specs --specs=nosys.specs \
               -Wl,--gc-sections -Wl,-Map=$(BUILD)/stm32/firmware.map
ARM_SRC     := $(CORE_SRC) platform/stm32c0/startup.c \
               platform/stm32c0/hal_stm32c0.c platform/stm32c0/main.c

.PHONY: help host run test stm32 clean

help:
	@echo "make test    build and run the unit tests"
	@echo "make host    build the PC version"
	@echo "make run     build and run the PC version"
	@echo "make stm32   build the STM32C031C6 firmware"
	@echo "make clean   remove the build folder"

host: $(HOST_BIN)

$(HOST_BIN): $(HOST_SRC) $(wildcard core/*.h)
	@mkdir -p $(dir $@)
	$(CC) $(HOST_CFLAGS) $(HOST_SRC) -o $@

run: host
	./$(HOST_BIN)

test: $(TEST_BINS)
	@for t in $(TEST_BINS); do $$t || exit 1; done
	@echo "All test programs passed."

$(BUILD)/tests/%: tests/%.c tests/fake_hal.c $(CORE_SRC) $(wildcard core/*.h) $(wildcard tests/*.h)
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) $< tests/fake_hal.c $(CORE_SRC) -o $@

stm32: $(BUILD)/stm32/firmware.elf $(BUILD)/stm32/firmware.bin

$(BUILD)/stm32/firmware.elf: $(ARM_SRC) $(wildcard core/*.h) $(wildcard platform/stm32c0/*.h) platform/stm32c0/linker.ld
	@mkdir -p $(dir $@)
	$(ARM_CC) $(ARM_CFLAGS) $(ARM_SRC) $(ARM_LDFLAGS) -o $@
	$(ARM_PREFIX)size $@

$(BUILD)/stm32/firmware.bin: $(BUILD)/stm32/firmware.elf
	$(ARM_PREFIX)objcopy -O binary $< $@

clean:
	rm -rf $(BUILD)
