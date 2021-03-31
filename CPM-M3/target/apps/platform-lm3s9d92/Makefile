CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy

DRI_DIR = ../dri-8k
INC_DIR = ../h

# turn off optimization because loop optimizer complains at a couple of
# places in stat and pip and I don't see what it's complaining about.
#
# I think it's complaining about spots that treat the filename and extent
# as if they were a single, larger array.

CFLAGS = -mthumb -mcpu=cortex-m3 -fno-builtin -nostdinc -fpack-struct -O0 -c

LDFLAGS = -T platform-lm3s9d92.ld

all::

#	Compile the DRI sources

	$(CC) $(CFLAGS) -I$(DRI_DIR) $(DRI_DIR)/stat.c
	$(CC) $(CFLAGS) -I$(DRI_DIR) $(DRI_DIR)/pip.c
	$(CC) $(CFLAGS) -I$(DRI_DIR) $(DRI_DIR)/ed.c

	$(CC) $(CFLAGS) -I$(DRI_DIR) $(DRI_DIR)/qsort.c

#	Compile non-dri sources

	$(CC) $(CFLAGS) oops.s

#	Compile our glue

	$(CC) $(CFLAGS) header.s
	$(CC) $(CFLAGS) setjmp.s
	$(CC) $(CFLAGS) -I$(INC_DIR) glue.c

#	linky linky

	$(LD) $(LDFLAGS) -Map stat.map -o stat.elf header.o glue.o \
	  stat.o qsort.o
	$(LD) $(LDFLAGS) -Map pip.map -o pip.elf header.o glue.o \
	  setjmp.o pip.o
	$(LD) $(LDFLAGS) -Map ed.map -o ed.elf header.o glue.o \
	  setjmp.o ed.o 

	$(LD) $(LDFLAGS) -Map oops.map -o oops.elf \
	  header.o oops.o

#	extract binaries

	$(OBJCOPY) -O binary stat.elf stat.com
	$(OBJCOPY) -O binary pip.elf pip.com
	$(OBJCOPY) -O binary ed.elf ed.com

	$(OBJCOPY) -O binary oops.elf oops.com

