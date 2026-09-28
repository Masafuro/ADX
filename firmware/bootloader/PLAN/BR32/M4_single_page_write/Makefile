# Makefile for Milestone 4 (M4): BR32 Single Flash Page Erase/Write Gate
# Target MCU: ATtiny1616-MNR

MCU          = attiny1616
F_CPU        = 3333333UL

TOOLCHAIN    = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin
CC           = $(TOOLCHAIN)/avr-gcc
OBJCOPY      = $(TOOLCHAIN)/avr-objcopy
SIZE         = $(TOOLCHAIN)/avr-size

OPTIMIZE     = -Os -fno-split-wide-types -mrelax
CFLAGS       = -g -Wall $(OPTIMIZE) -mmcu=$(MCU) -DF_CPU=$(F_CPU)
LDFLAGS      = -Wl,--relax

TARGET_ELF   = m4_flash.elf
TARGET_HEX   = m4_flash.hex

all: $(TARGET_HEX) check_size

$(TARGET_ELF): m4_flash.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

$(TARGET_HEX): $(TARGET_ELF)
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

check_size: $(TARGET_ELF)
	@echo "=== Binary Size ==="
	$(SIZE) -C --mcu=$(MCU) $(TARGET_ELF)

clean:
	rm -f $(TARGET_ELF) $(TARGET_HEX)

.PHONY: all clean check_size
