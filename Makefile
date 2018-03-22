#
# $Id$
#

#.DEFAULT:
#.SUFFIXES: .c .o .hex .elf
#.PHONY:
.SECONDARY:

all: main.hex

CFLAGS+= -I. -Os -DF_CPU=16000000UL -mmcu=atmega328p --std=c99
#CFLAGS+= -Wall
#CFLAGS+= -save-temps
#CFLAGS+= -fno-unwind-tables -fno-asynchronous-unwind-tables
CFLAGS+= -ffunction-sections -fdata-sections  -Wl,--gc-sections
#CFLAGS+= -MD -MP -MT $(*F).o -MF $(*F).d

LDFLAGS+= -s -DF_CPU=16000000UL -mmcu=atmega328p -lm
LDFLAGS+= -Wl,-u,vfprintf -lprintf_flt

MAIN_OBJS+= main.o fifo.o tools.o shell.o
MAIN_OBJS+= twim.o
MAIN_OBJS+= uart.o
MAIN_OBJS+= ds3231.o
MAIN_OBJS+= st7735.o
MAIN_OBJS+= at24c.o
MAIN_OBJS+= console.o
MAIN_OBJS+= ir.o
#MAIN_OBJS+= utime.o

main.elf: $(MAIN_OBJS)
	avr-gcc $(LDFLAGS) -o $@ $(^F)
	avr-size --format=berkeley $@

%.o: %.c
	avr-gcc $(CFLAGS) -c -o $@ $<

%.elf: %.o
	avr-gcc $(LDFLAGS) -o $@ $<

%.hex: %.elf
	avr-objcopy -O ihex -R .eeprom $< $@


%.upl: %.hex
	avrdude -qq -c arduino -p ATMEGA328P -P /dev/ttyU0 -b 115200 -U flash:w:$<

upload: main.upl

backup:
	avrdude -F -V -c arduino -p ATMEGA328P -P /dev/ttyU0 -b 57600 -U flash:r:backup.hex:i

clean:
	rm -f *.i *.o *.s *.elf *~ *.hex *.d

#EOF
