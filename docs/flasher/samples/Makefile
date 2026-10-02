# Makefile for ADX Sample Applications (Target: 0x1000)
# Copyright (c) 2026 ADX Project Contributors
# SPDX-License-Identifier: MIT

CC = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-gcc
OBJCOPY = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-objcopy
SIZE = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-size

MCU = attiny1616
F_CPU = 20000000UL
CFLAGS = -g -Wall -Os -fno-split-wide-types -mrelax -mmcu=$(MCU) -DF_CPU=$(F_CPU) -I../../../firmware/sandbox/M_firmware/inc
LDFLAGS = -Wl,-Ttext=0x1000 -Wl,--relax

SAMPLES = sample1_red_sos sample2_white_strobe sample3_smart_echo

all: $(addsuffix .bin, $(SAMPLES)) $(addsuffix .hex, $(SAMPLES))

%.elf: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

%.hex: %.elf
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

%.raw.bin: %.elf
	$(OBJCOPY) -O binary $< $@

%.bin: %.raw.bin
	python3 -c "with open('$<', 'rb') as f: data = f.read(); img = data.ljust(192 * 64, b'\xff'); open('$@', 'wb').write(img)"
	@echo "=== Built $@ (12KB / 192 Pages) ==="
	@ls -l $@

clean:
	rm -f *.elf *.hex *.raw.bin *.bin

.PHONY: all clean
