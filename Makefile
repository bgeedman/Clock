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

TARGET = clock

# Define the compiler we will be using
CC = avr-gcc

# Sets up the device name to be used during compilation
MCU = atmega16

# Processor frequency. This needs to be defined for several
# of the libraries to compile without warnings.
F_CPU = 16000000

# Level of optimization. 0, 1, 2, 3, s
# s is for size
OPT = 2

# If I add more source files, need to create individual objs
OBJDIR = .
SRC = $(TARGET).c

# Compiler flag for the C standard level. Not currently used
CSTANDARD = -std=c11

# math library linkage if needed. Not currently used
MATH_LIB = -lm

# Compiler flags to pass
FLAGS = -Wall -O$(OPT) -mmcu=$(MCU) -DF_CPU=$(F_CPU) $(CSTANDARD)

# AVR tool to create object file
AVRCOPY = avr-objcopy

# -j to copy those sections, -O for output format
COPY_FLAGS = -j .text -j .data -O ihex

# Define the programming tool to use
AVRDUDE = avrdude

# To get list of PARTNO use: avrdude -p ?
PARTNO = m16

# To get list of PROGRAMMER_ID use: avrdude -c ?
PROGRAMMER_ID = dragon_isp

# When using usb as port, it will search for the programmer
PORT = usb

# Tell avrdude where to write the code
WRITE_FLASH = -U flash:w:$(TARGET).hex

PROGRAM_FLAGS = -p $(PARTNO) -c $(PROGRAMMER_ID) -P $(PORT) $(WRITE_FLASH) -V





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
	$(CC) $(FLAGS) -o $(TARGET) $(TARGET).c

hex:
	$(AVRCOPY) $(COPY_FLAGS) $(TARGET) $(TARGET).hex

end:
	@echo "You will still need to run: make program"
	@echo "========================================"
	@echo

program:
	@echo "Uploading to device..."
	$(AVRDUDE) $(PROGRAM_FLAGS)
	@echo "Complete!"
	@echo

clean:
	@echo "========================================"
	@echo "Cleaning up"
	rm -f $(TARGET)
	rm -f $(TARGET).hex
	@echo "========================================"


.PHONY: clean
