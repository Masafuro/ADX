# Makefile for Milestone 3-1 (M3-1): Hardware Diag & Telemetry Firmware
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

TARGET_ELF   = m3_1_diag.elf
TARGET_HEX   = m3_1_diag.hex

ECHO_ELF     = echo_test.elf
ECHO_HEX     = echo_test.hex

all: $(TARGET_HEX) $(ECHO_HEX) check_size

$(TARGET_ELF): m3_1_diag.c ../../M_firmware/inc/mr32.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

$(TARGET_HEX): $(TARGET_ELF)
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

$(ECHO_ELF): echo_test.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

$(ECHO_HEX): $(ECHO_ELF)
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

check_size: $(TARGET_ELF) $(ECHO_ELF)
	@echo "=== Binary Size (Diag) ==="
	$(SIZE) -C --mcu=$(MCU) $(TARGET_ELF)
	@echo "=== Binary Size (Echo) ==="
	$(SIZE) -C --mcu=$(MCU) $(ECHO_ELF)

clean:
	rm -f $(TARGET_ELF) $(TARGET_HEX) $(ECHO_ELF) $(ECHO_HEX)

.PHONY: all clean check_size
