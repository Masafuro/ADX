# Makefile for Optiboot_OL4 (1-to-1 LN-485 High-Reliability Bootloader)
# Target MCU: ATtiny1616-MNR (BOOTEND = 0x04 -> 1024 Bytes)

MCU          = attiny1616
CC           = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-gcc
OBJCOPY      = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-objcopy
SIZE         = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-size

OPTIMIZE     = -Os -fno-split-wide-types -mrelax
CFLAGS       = -g -Wall $(OPTIMIZE) -mmcu=$(MCU)

LDFLAGS      = -Wl,-section-start=.text=0 -Wl,--section-start=.version=0x3fe
LDFLAGS     += -Wl,--relax -nostartfiles -nostdlib

TARGET_ELF   = optiboot_ol4_core_d.elf
TARGET_HEX   = optiboot_ol4_core_d.hex
BLANK_ELF    = blank_app.elf
BLANK_HEX    = blank_app.hex
COMBINED_HEX = optiboot_ol4_with_blank.hex

RELEASE_DIR  = ../releases

all: $(TARGET_HEX) $(COMBINED_HEX) check_size

$(TARGET_ELF): optiboot_ol4.c ln485_protocol.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

$(TARGET_HEX): $(TARGET_ELF)
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O ihex $< $@
	mkdir -p $(RELEASE_DIR)
	cp $@ $(RELEASE_DIR)/$@

$(BLANK_ELF): blank_app.c
	$(CC) -mmcu=$(MCU) -Os -nostartfiles -Wl,-section-start=.text=0x400 -o $@ $<

$(BLANK_HEX): $(BLANK_ELF)
	$(OBJCOPY) -O ihex $< $@

$(COMBINED_HEX): $(TARGET_HEX) $(BLANK_HEX)
	python3 merge_hex.py $(TARGET_HEX) $(BLANK_HEX) $@
	mkdir -p $(RELEASE_DIR)
	cp $@ $(RELEASE_DIR)/$@

check_size: $(TARGET_ELF)
	@echo "================== OPTIBOOT_OL4 BINARY SIZE =================="
	@$(SIZE) -A $(TARGET_ELF)
	@echo "=============================================================="

clean:
	rm -f $(TARGET_ELF) $(TARGET_HEX) $(BLANK_ELF) $(BLANK_HEX) $(COMBINED_HEX)
