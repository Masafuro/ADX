# Makefile for ADX Core-D 1-to-1 RS-485 Optiboot Bootloader
# Target MCU: ATtiny1616-MNR

MCU          = attiny1616
F_CPU        = 16000000UL
BAUD_RATE    = 115200
UARTTX       = A1
RS485        = 1
LED          = B2
WDTTIME      = 8

CC           = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-gcc
OBJCOPY      = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-objcopy
SIZE         = /home/ubuntu/.arduino15/packages/DxCore/tools/avr-gcc/7.3.0-atmel3.6.1-azduino7b1/bin/avr-size

OPTIMIZE     = -Os -fno-split-wide-types -mrelax
CFLAGS       = -g -Wall $(OPTIMIZE) -mmcu=$(MCU)
CFLAGS      += -DF_CPU=$(F_CPU) -DBAUD_RATE=$(BAUD_RATE)
CFLAGS      += -DUARTTX=$(UARTTX) -DRS485=$(RS485) -DLED=$(LED) -DWDTTIME=$(WDTTIME)
CFLAGS      += -DBLINK_WAIT_LED -DRS485_RE_PORT=VPORTA '-DRS485_RE_PIN=(1<<PORT7)'
CFLAGS      += -DENTRYCOND_REQUIRE=0x37

LDFLAGS      = -Wl,-section-start=.text=0 -Wl,--section-start=.version=0x1fe
LDFLAGS     += -Wl,--relax -nostartfiles -nostdlib

TARGET_ELF   = optiboot_core_d_rs485.elf
TARGET_HEX   = optiboot_core_d_rs485.hex
BLANK_ELF    = blank_app.elf
BLANK_HEX    = blank_app.hex
COMBINED_HEX = optiboot_core_d_with_blank.hex

TEST_WHITE_ELF     = test_blink_white_led.elf
TEST_WHITE_HEX     = test_blink_white_led.hex
TEST_ALT_ELF       = test_blink_alternate_led.elf
TEST_ALT_HEX       = test_blink_alternate_led.hex
TEST_SERIAL_ELF    = test_rs485_serial.elf
TEST_SERIAL_HEX    = test_rs485_serial.hex

RELEASE_DIR  = ../releases

all: $(TARGET_HEX) $(COMBINED_HEX) $(TEST_WHITE_HEX) $(TEST_ALT_HEX) $(TEST_SERIAL_HEX) check_size

$(TARGET_ELF): optiboot_x.c pin_defs_x.h stk500.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

$(TARGET_HEX): $(TARGET_ELF)
	$(OBJCOPY) -j .text -j .data -j .version --set-section-flags .version=alloc,load -O ihex $< $@
	mkdir -p $(RELEASE_DIR)
	cp $@ $(RELEASE_DIR)/$@

$(BLANK_ELF): blank_app.c
	$(CC) -mmcu=$(MCU) -Os -nostartfiles -Wl,-section-start=.text=0x200 -o $@ $<

$(BLANK_HEX): $(BLANK_ELF)
	$(OBJCOPY) -O ihex $< $@

$(COMBINED_HEX): $(TARGET_HEX) $(BLANK_HEX)
	python3 merge_hex.py $(TARGET_HEX) $(BLANK_HEX) $@
	cp $@ $(RELEASE_DIR)/$@

$(TEST_WHITE_ELF): test_blink_white_led.c
	$(CC) -mmcu=$(MCU) -Os -DF_CPU=3333333UL -nostartfiles -Wl,-section-start=.text=0x200 -o $@ $<

$(TEST_WHITE_HEX): $(TEST_WHITE_ELF)
	$(OBJCOPY) -O ihex $< $@
	mkdir -p $(RELEASE_DIR)
	cp $@ $(RELEASE_DIR)/$@

$(TEST_ALT_ELF): test_blink_alternate_led.c
	$(CC) -mmcu=$(MCU) -Os -DF_CPU=3333333UL -nostartfiles -Wl,-section-start=.text=0x200 -o $@ $<

$(TEST_ALT_HEX): $(TEST_ALT_ELF)
	$(OBJCOPY) -O ihex $< $@
	mkdir -p $(RELEASE_DIR)
	cp $@ $(RELEASE_DIR)/$@

$(TEST_SERIAL_ELF): test_rs485_serial.c
	$(CC) -mmcu=$(MCU) -Os -DF_CPU=3333333UL -nostartfiles -Wl,-section-start=.text=0x200 -o $@ $<

$(TEST_SERIAL_HEX): $(TEST_SERIAL_ELF)
	$(OBJCOPY) -O ihex $< $@
	mkdir -p $(RELEASE_DIR)
	cp $@ $(RELEASE_DIR)/$@

check_size: $(TARGET_ELF)
	@echo "================== BINARY SIZE CHECK =================="
	@$(SIZE) -A $(TARGET_ELF)
	@echo "======================================================="

clean:
	rm -f $(TARGET_ELF) $(TARGET_HEX) $(BLANK_ELF) $(BLANK_HEX) $(COMBINED_HEX) $(TEST_WHITE_ELF) $(TEST_WHITE_HEX) $(TEST_ALT_ELF) $(TEST_ALT_HEX) $(TEST_SERIAL_ELF) $(TEST_SERIAL_HEX)
