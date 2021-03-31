CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
CFLAGS = -mcpu=arm1176jz-s -fno-builtin -nostdinc -O1 -c
LDFLAGS =  -Wl,-T,platform-null.ld,--cref,-Map=nonicache.map 
LIB =
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
	$(CC) $(CFLAGS) $(NONDRIFLAGS) ../glue/arch-arm-icache.s
	$(CC) $(CFLAGS) $(NONDRIFLAGS) ../glue/arch-arm-noicache.s
	$(CC) $(CFLAGS) $(NONDRIFLAGS) ../glue/modubios.c
	$(CC) $(CFLAGS) $(NONDRIFLAGS) ../glue/consnull.c
	$(CC) $(CFLAGS) $(NONDRIFLAGS) ../glue/disknull.c

#	platform-specific code

	$(CC) $(CFLAGS) $(NONDRIFLAGS) platform.c

#	link non-icache variant

	$(CC) $(LDFLAGS) -o nonicache.elf \
	  platform.o bdosmain.o bdosmisc.o bdosrw.o ccp.o conbdos.o \
	  dskutil.o fileio.o iosys.o glue.o calledbydri.o \
	  arch-arm-noicache.o modubios.o consnull.o disknull.o $(LIB)

#	link icache variant

	$(CC) $(LDFLAGS) -o icache.elf \
	  platform.o bdosmain.o bdosmisc.o bdosrw.o ccp.o conbdos.o \
	  dskutil.o fileio.o iosys.o glue.o calledbydri.o \
	  arch-arm-noicache.o modubios.o consnull.o disknull.o $(LIB)

