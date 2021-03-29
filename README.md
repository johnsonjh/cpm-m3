# CP/M-M3

## Summary

This is a port of **CP/M-68K** to the **ARM Cortex-M3**, specifically, the
*LM3S9D92*. It builds under Windows. It is not quite ready for prime time in
that there are few applications and no documentation.

## Overview

Currently, the only supported hardware is the *LM3S9D92* microcontroller.
Personally, I run it on an *EK-LM3S9D92* development kit. Among other things,
the microcontroller includes a **Cortex-M3** processor, 512K of flash, 96K of
RAM, and a UART; other resources are not currently used by the system.

### Disk

The internal flash is used as a disk; essentially, **CP/M-M3** considers the
microcontroller to have a built-in 512K floppy. The disk is organized around
4K tracks, matching the erase block size of the flash. Well, technically, the
flash has 1K erase block, but there are enough interactions between the blocks
that it is easier to consider it as having a 4K erase block. The erase block
is managed by having a track cache; the cache is flushed (erasing and
reprogramming the flash block) at warm boot or when a disk write requires a
different track to be cached.

The first 8 tracks (32KB) are reserved to hold the operating system. At boot
time, the processor enters the bootstrap in the reserved area. The bootstrap
copies the system in RAM and jumps to it, just like a normal system would.

### RAM

The *TPA* resides in the first 64K of RAM, with the upper 32K reserved to
hold the system. The base page lives at the bottom of the *TPA* and occupies
the traditional 256 bytes. The entry point of a transient program is passed a
pointer to the base page; the program should use this pointer instead of 
a priori knowledge of the base page location to allow the program to be
easily migrated to other **CP/M-M3** systems with a different memory layout.

The *BDOS* is called through a function pointer located in the base page.
The current system does not provide a mechanism to call the system via a
*TRAP* instruction. A typical "hello world!" program looks something
like this:

```text
#include "cpm.h"
void Entry( cpm_basepage_t *BasePage )
{
  BasePage->bdos( 9, (unsigned int)"hello world!\r\n$" );
  return;
}
```
*DISCLAIMER: Off-the-cuff code; I haven't run it!*

The stack is placed at the top of the *TPA* and grows downward in the
usual manner. System calls execute on the transient program's stack.

### Executable Program Format

Executables are a simple memory image like a **CP/M-80** .*COM* file. 
Unlike a .*COM* file, they are expected to begin with a 16-byte header
containing:

* A reserved 32-bit longword intended to hold a branch around the header.
* A 32-bit longword containing a magic number that identifies the processor
  architecture for which the program is intended. For **CP/M-M3**, this
  magic number is *0x0000334d* (i.e., the string "*M3*").
* A 32-bit longword containing the address at which the program expects to be
  loaded; that is, the base address of the *TPA* plus 256 (for the base page).
  For the *LM3S9D92* port, this longword is expected to contain *0x20000100*.
* A 32-bit longword containing a function pointer that refers to the program's
  entry point.

**NOTE:** The .*bss* section is typically omitted by a C compiler. Since it
is not included in the memory image, it is not initialized by **CP/M-M3.**
The program will need to make its own arrangements to initialize the .*bss*
section, if that is necessary.

## Building the system

The system builds on Windows.

You'll need the following:

* Visual Studio for the host tools. I seem to be using Visual Studio
  Express 2013.
* Code Sourcery for the target programs. I'm using Sourcery CodeBench
  Lite for ARM EABI, version 2013.11-24.

The system is built by executing the `build.bat` command procedure in the
distribution directory in a command window that has executed the
`vcvarsall.bat` (or whatever they're calling it this week) command procedure
that prepares the environment for command-line usage of the Visual Studio
tools.

If all goes well, the following items are placed in the build directory:

* `cpm.bin`, the operating system image.
* `dim.exe`, a Windows program that allows manipulation of the disk image
  (where "disk image" means the image to be programmed into the
  microcontroller's flash).
* `ed.com`, the venerable *ED* editor from **CP/M-8K**, recompiled for
  **CP/M-M3**.
* `oops.com`, a program that sets the registers to known values and causes
  an exception, used to verify that exception handling is working.
* `pip.com`, *PIP* from **CP/M-8K** compiled for **CP/M-M3**.
* `stat.com`, *STAT* from **CP/M-68K** compiled for **CP/M-M3**.

The disk image manipulator, `dim.exe`, is a captive version of **CP/M-68K**
built to run as an application under Windows. It manipulates the file
`romdisk.img`, which is the binary image that needs to be burned into the
microcontroller's flash. If the image does not exist, it is created.

The *CCP* of `dim.exe` has been extended to have a few custom commands:

* `PUTSYS` copies the operating system image from a named host file to the
  reserved tracks in the disk image.
* `IMPORT` copies a named host file into the disk image.
* `EXPORT` copies a named file from the disk image to a host file.
* `EXIT` leaves `dim`, writing the disk image to `romdisk.img`.

## Running the system

You need something that has:

* An *LM3S9D92* microcontroller.
* A means of attaching UART 0 to some sort of terminal (or terminal emulator)
  running at 115,200 baud.
* A means of burning `romdisk.img` into the microcontroller's internal flash.

I use the *EK-LM3S9D92* development kit. This kit includes a program called
*LM Flash Programmer* that allows an image to be burned, regardless of the
size limits of the included evaluation toolset.

## Conclusion

Sadly, the *LM3S9D92* processor has gone into "*not* *recommended* *for*
*new* *designs*" status, which probably makes the dev kits hard to come by.

 - *Roger Ivie*
