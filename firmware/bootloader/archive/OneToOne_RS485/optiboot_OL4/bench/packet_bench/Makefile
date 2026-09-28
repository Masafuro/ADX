# Makefile for Core-D RS-485 Packet Benchmark @ 9600 bps
MCU          = attiny1616
CC           = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-gcc
OBJCOPY      = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-objcopy
SIZE         = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-size

OPTIMIZE     = -Os -fno-split-wide-types -mrelax
CFLAGS       = -g -Wall $(OPTIMIZE) -mmcu=$(MCU)

LDFLAGS      = -Wl,-section-start=.text=0 -Wl,--relax -nostartfiles -nostdlib -lgcc

TARGET_ELF   = packet_bench.elf
TARGET_HEX   = packet_bench.hex

all: $(TARGET_HEX) check_size

$(TARGET_ELF): packet_bench.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

$(TARGET_HEX): $(TARGET_ELF)
	$(OBJCOPY) -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .userrow $< $@

check_size: $(TARGET_ELF)
	@echo "================== PACKET BENCH BINARY SIZE =================="
	@$(SIZE) -A $(TARGET_ELF)
	@echo "============================================================="

clean:
	rm -f $(TARGET_ELF) $(TARGET_HEX)
