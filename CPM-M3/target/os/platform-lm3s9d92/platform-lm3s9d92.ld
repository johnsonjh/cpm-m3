/*************************************************************
 *
 * file: platform-lm3s9d92.ld
 *
 *	This file is the ld script for CP/M running on an LM3S9D92.
 *
 *	We have:
 *
 *	- 512K of flash starting at 0.
 *
 *	- 96K of RAM starting at 0x20000000. We want CP/M at the top
 *	  of the RAM, but I'm a ld newbie and am not aware of a command
 *	  to put stuff at the end of a section. 
 *
 *	  So, as far as this ld script is concerned, we have 32K of RAM
 *	  starting at 0x20010000. 
 *
 *	We also need to define a few symbols for use by the bootstrap:
 *
 *	- bstrap_copy_from: The load address of the stuff that needs to
 *	  be copied to RAM.
 *
 *	- bstrap_copy_to: The RAM address it needs to be copied to.
 *
 *	- bstrap_copy_size: The size, in bytes, of the region that needs
 *	  to be copied.
 *
 *	- bstrap_zero_to: The RAM address of the region that needs to be
 *	  cleared.
 *
 *	- bstrap_zero_size: The size, in bytes, of the region that needs
 *	  to be zeroed.
 *
 *	- bstrap_top: The RAM address just above the system. This helps
 *	  in shoving the system to the top until it just fits.
 *
 * revisions:
 *
 *	2013-12-21 rli: original attempt.
 *
 *	2014-01-01 rli: looks like I can push up almost, but not quite, 8K.
 *
 **********************************************************/

SECTIONS
{
  bstrap 0x00000000 : {
    bootvectors.o(.text)
    bstrap.o(.text)
    bstrap.o(.rodata)
  }

  .text 0x20011e00 : AT( LOADADDR( bstrap ) + SIZEOF( bstrap ) ) {
    vectors.o(.text)
    *(.text)
    *(.rodata)
  }
  .data : ALIGN( 4 ) {
    *(.data)
    *(.image)
  }
  .bss : ALIGN( 4 ) {
    *(.bss)
    *(COMMON)
  }

  bstrap_copy_from = LOADADDR(.text);
  bstrap_copy_to = ADDR(.text);
  bstrap_copy_size = LOADADDR(.bss) - LOADADDR(.text);

  bstrap_zero_to = ADDR(.bss);
  bstrap_zero_size = SIZEOF(.bss);

  bstrap_top = ADDR(.bss) + SIZEOF(.bss);
  bstrap_totalsize = bstrap_top - ADDR(.text);
}
