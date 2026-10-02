# Makefile for WU-1: MR32 32-Byte Fixed-Length Frame Echo & Fast Reject Lab
# Target MCU: ATtiny1616-MNR on ADX Core-D

MCU          = attiny1616
F_CPU        = 20000000UL

TOOLCHAIN    = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin
CC           = $(TOOLCHAIN)/avr-gcc
OBJCOPY      = $(TOOLCHAIN)/avr-objcopy
SIZE         = $(TOOLCHAIN)/avr-size

OPTIMIZE     = -Os -fno-split-wide-types -mrelax
CFLAGS       = -g -Wall $(OPTIMIZE) -mmcu=$(MCU) -DF_CPU=$(F_CPU) -I../../M_firmware/inc
LDFLAGS      = -Wl,--relax

TARGET_ELF   = wu1_fast_reject.elf
TARGET_HEX   = wu1_fast_reject.hex

all: $(TARGET_HEX) check_size

$(TARGET_ELF): wu1_fast_reject.c ../../M_firmware/inc/mr32.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

$(TARGET_HEX): $(TARGET_ELF)
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

check_size: $(TARGET_ELF)
	@echo "=== Binary Size ==="
	$(SIZE) -C --mcu=$(MCU) $(TARGET_ELF)

clean:
	rm -f $(TARGET_ELF) $(TARGET_HEX)

.PHONY: all clean check_size
