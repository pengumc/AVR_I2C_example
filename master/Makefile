# Name: Makefile
# Author: Michiel van der Coelen
# date: 2010-12-31
# tabsize: 2
 
MMCU = atmega88
AVRDUDE_MCU = m88
AVRDUDE_PROGRAMMERID = usbasp
#AVRDUDE_PORT = 
F_CPU = 20000000
NAME = I2Cmaster


OBJECTS = $(NAME).o 
CFLAGS = -DF_CPU=$(F_CPU) -std=c99 -Wall -Os -mmcu=$(MMCU) 
CC = avr-gcc
SIZE = avr-size
OBJCOPY = avr-objcopy

.PHONY: all clean test
all: $(NAME).hex
	$(SIZE) $(NAME).hex

#rebuild everything!
force: clean all

$(NAME).hex: $(NAME).elf
	$(OBJCOPY) -O ihex $(NAME).elf $(NAME).hex
	
$(NAME).elf: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(NAME).elf $(OBJECTS)

#compile src files
%.o: %.c
		$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@


clean:
	rm -f $(OBJECTS) $(NAME).elf

program: $(NAME).hex
	avrdude -c $(AVRDUDE_PROGRAMMERID) -p $(AVRDUDE_MCU) -U flash:w:$(NAME).hex

test:
	avrdude -c $(AVRDUDE_PROGRAMMERID) -p $(AVRDUDE_MCU) -v