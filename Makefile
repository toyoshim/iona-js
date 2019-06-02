# Copyright 2019, Takashi TOYOSHIMA <toyoshim@gmail.com> All rights reserved.
# Use of this source code is governed by a BSD-style license that can be found
# in the LICENSE file.

CC	= avr-g++
OBJCOPY	= avr-objcopy
MCU	= atmega32a
CFLAGS	= -Os -Wall -mmcu=$(MCU) -std=c++11 -I$(PWD)
LFLAGS	= -Wall -mmcu=$(MCU)
DUDEOPT	= -C ~/opt/etc/avrdude.conf -c usbtiny -pm32
TARGET	= iona-basic
OBJS	= JVSIO.o JVSIOClient.o Jamma.o Arduino.o main.o

all: $(TARGET).hex

clean:
	rm -rf *.o *.elf *.hex *.bin

size: $(TARGET).elf
	$(SIZE) $(SFLAGS) $(TARGET).elf

program: $(TARGET).hex
	avrdude $(DUDEOPT) -U flash:w:$<:i

# Fuse High - 1100 1001 - !OCDEN | !JTAGEN | SPIEN | CKOPT | !EESAVE | BOOT(APP)
# Fuse Low  - 1110 1111 - !BODLEVEL | !BODEN | SUT(10) | CLSEL(1111-EXT)
#   expects 16MHz external clock
fuse:
	avrdude $(DUDEOPT) -U lfuse:w:0xef:m -U hfuse:w:0xc9:m

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@

JVSIO.o: jvsio/JVSIO.cpp *.h
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.cpp *.h
	$(CC) -c $(CFLAGS) -o $@ $<

$(TARGET).elf: $(OBJS)
	$(CC) $(LFLAGS) -o $@ $^
