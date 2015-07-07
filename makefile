#NOTE: This file changes the fuse values of the chip to enable brown-out detection.
#fuse settings are hard-coded into the bottom lines; change them only with care.

PRG            = mivuuauad
SRC            = uart.c adc.c shift.c mivuuauad.c
OBJ            = ${SRC:.c=.o}
MCU_TARGET     = atmega8
PROGRAMMER     = usbasp
AVRDUDE_TARGET = atmega8
PORT           = usb
F_CPU          = 16000000
BAUD           = 38400

OPTIMIZE       = -Os -std=c99

DEFS           =
LIBS           = uart.c


# You should not have to change anything below here.

CC             = avr-gcc

# Override is only needed by avr-lib build system.

override CFLAGS        = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS) -DF_CPU=$(F_CPU) -DBAUD=$(BAUD)
override LDFLAGS       = -Wl,-Map,$(PRG).map

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump

all: $(PRG).elf lst text eeprom

.c.o:
	@echo " ** "$(CC) $(CFLAGS) $<
	@$(CC) -c $(CFLAGS) $<

$(PRG).elf: $(OBJ)
	@#$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
	@#$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
	@$(CC) -o $@ $(OBJ) $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf *.o $(PRG).elf *.eps *.png *.pdf *.bak *.hex *.bin *.srec
	rm -rf *.lst *.map $(EXTRA_CLEAN_FILES)

lst:  $(PRG).lst

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

# Rules for building the .text rom images

text: hex bin srec

hex:  $(PRG).hex
bin:  $(PRG).bin
srec: $(PRG).srec

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@

# Rules for building the .eeprom rom images

eeprom: ehex ebin esrec


ehex:  $(PRG)_eeprom.hex
ebin:  $(PRG)_eeprom.bin
esrec: $(PRG)_eeprom.srec

%_eeprom.hex: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@

%_eeprom.srec: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O srec $< $@

%_eeprom.bin: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O binary $< $@


# command to program chip (invoked by running "make program")-P $(PORT) -v -e -b 115200  
program: 
	avrdude -p $(AVRDUDE_TARGET) -c $(PROGRAMMER) \
	 -U lfuse:w:0xe4:m  -U hfuse:w:0xDF:m \
	 -U efuse:w:0xff:m -U eeprom:w:$(PRG)_eeprom.hex  \
	 -U flash:w:$(PRG).hex	
	  
	 
# Original fuse settings: 	
#	 -U lfuse:w:0x64:m  -U hfuse:w:0xDF:m	-U efuse:w:0xff:m	
	