CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy

CFLAGS = -mthumb -mcpu=cortex-m3 -fno-builtin -nostdinc -O1 -c
LDFLAGS = -Wl,-T,platform-lm3s9d92.ld,--cref,-Map=cpm.map
#LIB = -lgcc
#LIB = 
DRIFLAGS = -I. -I../dri-68k
NONDRIFLAGS = -I. -I../h

all::

#	the DRI stuff

	$(CC) $(CFLAGS) $(DRIFLAGS) ../dri-68k/bdosmain.c
	$(CC) $(CFLAGS) $(DRIFLAGS) ../dri-68k/bdosmisc.c
	$(CC) $(CFLAGS) $(DRIFLAGS) ../dri-68k/bdosrw.c
	$(CC) $(CFLAGS) $(DRIFLAGS) ../dri-68k/ccp.c
	$(CC) $(CFLAGS) $(DRIFLAGS) ../dri-68k/conbdos.c
	$(CC) $(CFLAGS) $(DRIFLAGS) ../dri-68k/dskutil.c
	$(CC) $(CFLAGS) $(DRIFLAGS) ../dri-68k/fileio.c
	$(CC) $(CFLAGS) $(DRIFLAGS) ../dri-68k/iosys.c

#	reasonably architecture-independent glue

	$(CC) $(CFLAGS) $(NONDRIFLAGS) ../glue/glue.c
	$(CC) $(CFLAGS) $(NONDRIFLAGS) ../glue/calledbydri.c
	$(CC) $(CFLAGS) $(NONDRIFLAGS) ../glue/arch-m3-noicache.s
	$(CC) $(CFLAGS) $(NONDRIFLAGS) ../glue/modubios.c
	$(CC) $(CFLAGS) $(NONDRIFLAGS) ../glue/consnull.c
	$(CC) $(CFLAGS) $(NONDRIFLAGS) ../glue/disknull.c

#	platform-specific code

	$(CC) $(CFLAGS) $(NONDRIFLAGS) platform.c
	$(CC) $(CFLAGS) $(NONDRIFLAGS) consuart0.c
	$(CC) $(CFLAGS) $(NONDRIFLAGS) diskflash.c
	$(CC) $(CFLAGS) $(NONDRIFLAGS) bootvectors.s
	$(CC) $(CFLAGS) $(NONDRIFLAGS) bstrap.c
	$(CC) $(CFLAGS) $(NONDRIFLAGS) exception.c
	$(CC) $(CFLAGS) $(NONDRIFLAGS) vectors.s

#	link non-icache variant

#	$(LD) -N -T platform-lm3s9d92.ld --cref -o cpm.elf \
#	  -Map cpm.map \

	$(CC) $(LDFLAGS) -o cpm.elf \
	  bootvectors.o bstrap.o vectors.o \
	  platform.o bdosmisc.o bdosmain.o bdosrw.o ccp.o conbdos.o \
	  dskutil.o fileio.o iosys.o glue.o calledbydri.o \
	  arch-m3-noicache.o modubios.o consnull.o disknull.o \
	  consuart0.o diskflash.o exception.o 

#	make a binary image

	$(OBJCOPY) -O binary cpm.elf cpm.bin
