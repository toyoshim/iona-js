# Copyright 2019, Takashi TOYOSHIMA <toyoshim@gmail.com> All rights reserved.
# Use of this source code is governed by a BSD-style license that can be found
# in the LICENSE file.

# for PROTO PCB build that uses the internal RC 8MHz to run.
#CFLAGS	= -Os -Wall -mmcu=$(MCU) -std=c++11 -I$(PWD) -DPROTO
#CFLAGS	= -Os -Wall -mmcu=$(MCU) -std=c++11 -I$(PWD) -DALT_SWAP

# Default flags for Ver 1.01 PCB.
#CFLAGS	= -Os -Wall -mmcu=$(MCU) -std=c++11 -I$(PWD) -DNO_DEBUG

# Default flags for Ver 1.10 PCB.
CFLAGS	= -Os -Wall -mmcu=$(MCU) -std=c++11 -I$(PWD) -DNO_SWAP -DNO_DEBUG

CC	= avr-g++
OBJCOPY	= avr-objcopy
MCU	= atmega32a
LFLAGS	= -Wall -mmcu=$(MCU)
DUDEOPT	= -C ~/opt/etc/avrdude.conf -c usbtiny -pm32
TARGET	= iona-basic
OBJS	= JVSIO.o JVSIOClient.o Jamma.o DIPSW.o Arduino.o main.o

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
#   (PROTO) - 1110 0100 - !BODLEVEL | !BODEN | SUT(10) | CLSEL(0100-RC)
#   expects 8MHz RC internal clock
fuse:
	avrdude $(DUDEOPT) -U lfuse:w:0xef:m -U hfuse:w:0xc9:m

fuse-proto:
	avrdude $(DUDEOPT) -U lfuse:w:0xe4:m -U hfuse:w:0xc9:m

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
