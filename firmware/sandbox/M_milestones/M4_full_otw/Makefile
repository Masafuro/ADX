# Copyright (c) 2026 ADX Project Contributors
# SPDX-License-Identifier: MIT

CC = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-gcc
OBJCOPY = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-objcopy
SIZE = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-size

MCU = attiny1616
F_CPU = 20000000UL
CFLAGS = -g -Wall -Os -fno-split-wide-types -mrelax -mmcu=$(MCU) -DF_CPU=$(F_CPU) -I../../M_firmware/inc
LDFLAGS = -Wl,--relax

all: m4_bootloader.hex user_app.bin app_12k.bin size

m4_bootloader.elf: m4_bootloader.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

m4_bootloader.hex: m4_bootloader.elf
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

user_app.elf: user_app.c
	$(CC) $(CFLAGS) -Wl,-Ttext=0x1000 $(LDFLAGS) -o $@ $<

user_app.hex: user_app.elf
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

user_app.bin: user_app.elf
	$(OBJCOPY) -O binary $< $@

app_12k.bin: user_app.bin
	python3 -c "with open('user_app.bin', 'rb') as f: data = f.read(); img = data.ljust(192 * 64, b'\xff'); open('app_12k.bin', 'wb').write(img)"

size: m4_bootloader.elf user_app.elf
	@echo "=== Bootloader Binary Size ==="
	$(SIZE) -C --mcu=$(MCU) m4_bootloader.elf
	@echo "=== User App Binary Size ==="
	$(SIZE) -C --mcu=$(MCU) user_app.elf
	@echo "=== 12KB OTW Image Size ==="
	@ls -l app_12k.bin

clean:
	rm -f m4_bootloader.elf m4_bootloader.hex user_app.elf user_app.hex user_app.bin app_12k.bin

.PHONY: all size clean
