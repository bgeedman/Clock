# Makefile for the HDD clock.
#----------------------------------------------------------
# Usage:
# make all = compiles code and creates hex file
#
# make clean = removes all non code artifacts
#
# make elf = creates assembler code only
#
# make hex = creates the hex file to copy over
#
# make program = Download the hex file to the device
#----------------------------------------------------------

TARGET = main
CC = avr-gcc
MCU = atmega16
F_CPU = 16000000
OPT = 2
CSTANDARD = -std=gnu99
MATH_LIB = -lm
FLAGS = -Wall -O$(OPT) -mmcu=$(MCU) -DF_CPU=$(F_CPU)

AVRCOPY = avr-objcopy
COPY_FLAGS = -j .text -j .data -O ihex


all: begin gccversion elf copyversion hex end

default: all

begin:
	@echo
	@echo "========================================"
	@echo "Beginnning compilation process"


gccversion:
	$(CC) --version
	@echo

copyversion:
	$(AVRCOPY) --version
	@echo

elf:
	$(CC) $(FLAGS) -o $(TARGET) main.c

hex: main
	$(AVRCOPY) $(COPY_FLAGS) main main.hex


end:
	@echo "You will still need to run: make program"
	@echo "========================================"
	@echo

clean:
	@echo "========================================"
	@echo "Cleaning up"
	rm -f main
	rm -f main.hex
	@echo "========================================"


.PHONY: clean
