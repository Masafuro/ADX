# Makefile for Simple RS-485 Byte Echo Probe
MCU          = attiny1616
CC           = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-gcc
OBJCOPY      = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-objcopy
SIZE         = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-size

OPTIMIZE     = -Os -fno-split-wide-types -mrelax
CFLAGS       = -g -Wall $(OPTIMIZE) -mmcu=$(MCU)

LDFLAGS      = -Wl,-section-start=.text=0 -Wl,--relax -nostartfiles -nostdlib -lgcc

TARGET_ELF   = simple_echo.elf
TARGET_HEX   = simple_echo.hex

all: $(TARGET_HEX) check_size

$(TARGET_ELF): simple_echo.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

$(TARGET_HEX): $(TARGET_ELF)
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .userrow $< $@

check_size: $(TARGET_ELF)
	@echo "================== SIMPLE ECHO BINARY SIZE =================="
	@$(SIZE) -A $(TARGET_ELF)
	@echo "============================================================="

clean:
	rm -f $(TARGET_ELF) $(TARGET_HEX)
